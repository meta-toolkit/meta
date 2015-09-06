/**
 * @file language_model.cpp
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <sstream>
#include <random>
#include "util/time.h"
#include "util/shim.h"
#include "lm/language_model.h"
#include "logging/logger.h"

namespace meta
{
namespace lm
{

language_model::language_model(const cpptoml::table& config)
{
    auto table = config.get_table("language-model");
    auto arpa_file = table->get_as<std::string>("arpa-file");
    auto binary_file = table->get_as<std::string>("binary-file-prefix");

    N_ = 0;
    if (binary_file && filesystem::file_exists(*binary_file + "0.binlm"))
    {
        LOG(info) << "Loading language model from binary files: "
                  << *binary_file << "*" << ENDLG;
        auto time = common::time(
            [&]()
            {
                while (filesystem::file_exists(*binary_file + std::to_string(N_)
                                               + ".binlm"))
                    lm_.emplace_back(*binary_file + std::to_string(N_++)
                                     + ".binlm");
            });
        LOG(info) << "Done. (" << time.count() << "ms)" << ENDLG;
        prefix_ = *binary_file;
    }
    else if (arpa_file && binary_file)
    {
        LOG(info) << "Loading language model from .arpa file: " << *arpa_file
                  << ENDLG;
        prefix_ = *binary_file;
        auto time = common::time([&]()
                                 {
                                     read_arpa_format(*arpa_file);
                                 });
        LOG(info) << "Done. (" << time.count() << "ms)" << ENDLG;
    }
    else
        throw language_model_exception{
            "arpa-file or binary-file-prefix needed in config file"};

    load_vocab();
}

void language_model::read_arpa_format(const std::string& arpa_file)
{
    std::ifstream infile{arpa_file};
    std::string buffer;

    // get to beginning of unigram data, saving the counts of each ngram type
    std::vector<uint64_t> count;
    while (std::getline(infile, buffer))
    {
        if (buffer.find("ngram ") == 0)
        {
            auto equal = buffer.find_first_of("=");
            count.emplace_back(std::stoi(buffer.substr(equal + 1)));
        }

        if (buffer.find("\\1-grams:") == 0)
            break;
    }

    lm_.emplace_back(prefix_ + std::to_string(N_) + ".binlm", count[N_]);
    std::ofstream unigrams{prefix_ + "0.strings"};
    while (std::getline(infile, buffer))
    {
        // if blank or end
        if (buffer.empty() || (buffer[0] == '\\' && buffer[1] == 'e'))
            continue;

        // if start of new ngram data
        if (buffer[0] == '\\')
        {
            ++N_;
            lm_.emplace_back(prefix_ + std::to_string(N_) + ".binlm",
                             count[N_]);
            continue;
        }

        auto first_tab = buffer.find_first_of('\t');
        float prob = std::stof(buffer.substr(0, first_tab));
        auto second_tab = buffer.find_first_of('\t', first_tab + 1);
        auto ngram = buffer.substr(first_tab + 1, second_tab - first_tab - 1);
        float backoff = 0.0;
        if (second_tab != std::string::npos)
            backoff = std::stof(buffer.substr(second_tab + 1));
        lm_[N_].insert(ngram, prob, backoff);

        if (N_ == 0)
            unigrams << ngram << std::endl;
    }

    ++N_;
}

std::vector<std::pair<std::string, float>>
    language_model::top_k(const sentence& prev, size_t k) const
{
    // this is horribly inefficient due to this LM's structure
    using pair_t = std::pair<std::string, float>;
    auto comp = [](const pair_t& a, const pair_t& b)
    {
        return a.second > b.second;
    };
    std::vector<pair_t> candidates;

    sentence candidate = prev;
    candidate.push_back("word"); // the last item is replaced each iteration
    for (const auto& word : vocabulary_)
    {
        auto candidate = sentence{prev.to_string() + " " + word};
        candidates.emplace_back(word, log_prob(candidate));
        std::push_heap(candidates.begin(), candidates.end(), comp);
        if (candidates.size() > k)
        {
            std::pop_heap(candidates.begin(), candidates.end(), comp);
            candidates.pop_back();
        }
    }

    for (auto end = candidates.end(); end != candidates.begin(); --end)
        std::pop_heap(candidates.begin(), end, comp);

    return candidates;
}

void language_model::load_vocab()
{
    std::string word;
    std::ifstream unigrams{prefix_ + "0.strings"};
    while (std::getline(unigrams, word))
    {
        if (word.empty())
            continue;

        vocabulary_.push_back(word);
    }
}

float language_model::prob_calc(sentence tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{"prob_calc: tokens is empty!"};

    if (tokens.size() == 1)
    {
        auto opt = lm_[0].find(tokens[0]);
        if (opt)
            return opt->prob;
        return lm_[0].find("<unk>")->prob;
    }
    else
    {
        auto opt = lm_[tokens.size() - 1].find(tokens.to_string());
        if (opt)
            return opt->prob;

        auto hist = tokens(0, tokens.size() - 1);
        tokens.pop_front();
        if (tokens.size() == 1)
        {
            hist = hist(0, 1);
            auto opt = lm_[0].find(hist[0]);
            if (!opt)
                hist.substitute(0, "<unk>");
        }

        opt = lm_[hist.size() - 1].find(hist.to_string());
        if (opt)
            return opt->backoff + prob_calc(tokens);
        return prob_calc(tokens);
    }
}

float language_model::log_prob(sentence tokens) const
{
    float prob = 0.0f;

    // tokens < N
    sentence ngram;
    for (uint64_t i = 0; i < N_ - 1 && i < tokens.size(); ++i)
    {
        ngram.push_back(tokens[i]);
        prob += prob_calc(ngram);
    }

    // tokens >= N
    for (uint64_t i = N_ - 1; i < tokens.size(); ++i)
    {
        ngram.push_back(tokens[i]);
        prob += prob_calc(ngram);
        ngram.pop_front();
    }

    return prob;
}

float language_model::perplexity(const sentence& tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{"perplexity() called on empty sentence"};
    return std::pow(10.0, -(log_prob(tokens) / tokens.size()));
}

float language_model::perplexity_per_word(const sentence& tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{
            "perplexity_per_word() called on empty sentence"};
    return perplexity(tokens) / tokens.size();
}
}
}
