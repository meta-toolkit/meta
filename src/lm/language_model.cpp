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
#include "util/fixed_heap.h"
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
                prefix_ = *binary_file;
                load_vocab();
                while (filesystem::file_exists(*binary_file + std::to_string(N_)
                                               + ".binlm"))
                    lm_.emplace_back(*binary_file + std::to_string(N_++)
                                     + ".binlm");
            });
        LOG(info) << "Done. (" << time.count() << "ms)" << ENDLG;
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

    // cache this value
    auto unk = vocabulary_.at("<unk>");
    unk_node_ = *lm_[0].find(&unk, &unk + 1);
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
    term_id unigram_id{0};
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

        if (N_ == 0)
        {
            unigrams << ngram << std::endl;
            vocabulary_.emplace(ngram, unigram_id++);
        }

        lm_[N_].insert(token_list{ngram, vocabulary_}, prob, backoff);
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
    util::fixed_heap<pair_t, decltype(comp)> candidates{k, comp};

    token_list candidate{prev, vocabulary_};
    candidate.push_back(0_tid);
    for (const auto& word : vocabulary_)
    {
        candidate[candidate.size() - 1] = word.second;
        candidates.emplace(word.first, log_prob(candidate));
    }

    return candidates.extract_top();
}

void language_model::load_vocab()
{
    std::string word;
    std::ifstream unigrams{prefix_ + "0.strings"};
    term_id cur{0};
    while (std::getline(unigrams, word))
    {
        if (word.empty())
            continue;

        vocabulary_.emplace(word, cur++);
    }
}

float language_model::prob_calc(token_list tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{"prob_calc: tokens is empty!"};

    if (tokens.size() == 1)
    {
        auto opt = lm_[0].find(token_list{tokens[0]});
        if (opt)
            return opt->prob;
        return unk_node_.prob;
    }
    else
    {
        auto opt = lm_[tokens.size() - 1].find(tokens);
        if (opt)
            return opt->prob;

        auto hist = tokens;
        hist.pop_back();
        tokens.pop_front();
        if (tokens.size() == 1)
        {
            auto opt = lm_[0].find(hist[0]);
            if (!opt)
                hist[0] = vocabulary_.at("<unk>");
        }

        opt = lm_[hist.size() - 1].find(hist);
        if (opt)
            return opt->backoff + prob_calc(tokens);
        return prob_calc(tokens);
    }
}

float language_model::log_prob(const sentence& tokens) const
{
    return log_prob(token_list{tokens, vocabulary_});
}

float language_model::log_prob(const token_list& tokens) const
{
    using diff_type = decltype(tokens.tokens().begin())::difference_type;
    float prob = 0.0f;

    // tokens < N
    for (uint64_t i = 0; i < N_ - 1 && i < tokens.size(); ++i)
    {
        prob += prob_calc(tokens.tokens().begin(),
                          tokens.tokens().begin() + static_cast<diff_type>(i)
                              + 1);
    }

    // tokens >= N
    for (uint64_t i = N_ - 1; i < tokens.size(); ++i)
    {
        prob += prob_calc(
            tokens.tokens().begin() + static_cast<diff_type>(i - N_ + 1),
            tokens.tokens().begin() + static_cast<diff_type>(i) + 1);
    }

    return prob;
}

float language_model::perplexity(const sentence& tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{"perplexity() called on empty sentence"};
    return std::pow(10.0f, -(log_prob(tokens) / tokens.size()));
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
