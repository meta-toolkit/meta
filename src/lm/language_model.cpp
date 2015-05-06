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
}

std::string language_model::next_token(const sentence& tokens,
                                       double random) const
{
    throw language_model_exception{"could not generate next token: "
                                   + tokens.to_string()};
}

std::vector<std::pair<std::string, double>>
    language_model::top_k(const sentence& prev, size_t k) const
{
}

std::string language_model::generate(unsigned int seed) const
{
    return "";
}

double language_model::prob(sentence tokens) const
{
    return 0.0;
}

double language_model::perplexity(const sentence& tokens) const
{
    sentence ngram;
    for (size_t i = 1; i < N_; ++i)
        ngram.push_back("<s>");

    double perp = 0.0;
    for (auto& token : tokens)
    {
        ngram.push_back(token);
        perp += std::log(1.0 / prob(ngram));
        ngram.pop_front();
    }

    return perp / N_;
}

double language_model::perplexity_per_word(const sentence& tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{
            "perplexity_per_word called on empty sentence"};
    return perplexity(tokens) / tokens.size();
}
}
}
