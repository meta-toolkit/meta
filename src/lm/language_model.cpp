/**
 * @file language_model.cpp
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <iostream>
#include <sstream>
#include <random>
#include "util/shim.h"
#include "lm/language_model.h"

namespace meta
{
namespace lm
{

language_model::language_model(const cpptoml::table& config)
{
    auto table = config.get_table("language-model");
    auto arpa_file = table->get_as<std::string>("arpa-file");
    read_arpa_format(*arpa_file);
}

void language_model::read_arpa_format(const std::string& arpa_file)
{
    std::ifstream infile{arpa_file};
    std::string buffer;

    // get to beginning of unigram data
    while (std::getline(infile, buffer))
    {
        if (buffer.find("\\1-grams:") == 0)
            break;
    }

    N_ = 0;

    while (std::getline(infile, buffer))
    {
        if (buffer.empty())
            continue;

        if (buffer[0] == '\\')
        {
            ++N_;
            continue;
        }

        auto first_tab = buffer.find_first_of('\t');
        float prob = std::stof(buffer.substr(0, first_tab));
        auto second_tab = buffer.find_first_of('\t', first_tab + 1);
        auto ngram = buffer.substr(first_tab + 1, second_tab - first_tab - 1);
        float backoff = 0.0;
        if (second_tab != std::string::npos)
            backoff = std::stof(buffer.substr(second_tab + 1));
        lm_[ngram] = {prob, backoff};
    }
}

std::string language_model::next_token(const sentence& tokens,
                                       double random) const
{
    throw language_model_exception{"could not generate next token: "
                                   + tokens.to_string()};
}

std::vector<std::pair<std::string, float>>
    language_model::top_k(const sentence& prev, size_t k) const
{
}

std::string language_model::generate(unsigned int seed) const
{
    return "";
}

float language_model::prob_calc(sentence tokens) const
{
    if (tokens.size() == 1)
    {
        auto it = lm_.find(tokens[0]);
        if (it != lm_.end())
            return it->second.prob;
        return lm_.at("<unk>").prob;
    }
    else
    {
        auto it = lm_.find(tokens.to_string());
        if (it != lm_.end())
            return it->second.prob;

        auto hist = tokens(0, tokens.size() - 1);
        tokens.pop_front();
        if (tokens.size() == 1)
        {
            hist = hist(0, 1);
            auto it = lm_.find(hist[0]);
            if (it == lm_.end())
                hist.substitute(0, "<unk>");
        }

        it = lm_.find(hist.to_string());
        if (it != lm_.end())
            return it->second.backoff + prob_calc(tokens);
        return prob_calc(tokens);
    }
}

float language_model::log_prob(sentence tokens) const
{
    tokens.push_front("<s>");
    tokens.push_back("</s>");
    float prob = 0.0f;

    // tokens < N
    sentence ngram;
    for (uint64_t i = 0; i < N_ - 1; ++i)
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
    return std::pow(
        10.0, -(log_prob(tokens) / (tokens.size() + 2))); // +2 for <s> and </s>
}

float language_model::perplexity_per_word(const sentence& tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{
            "perplexity_per_word() called on empty sentence"};
    return perplexity(tokens) / (tokens.size() + 2); // +2 for <s> and </s>
}
}
}
