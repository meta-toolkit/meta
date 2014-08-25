/**
 * @file language_model.tcc
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <iostream>
#include <sstream>
#include <random>
#include "analyzers/analyzer.h"
#include "analyzers/tokenizers/icu_tokenizer.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/filters/alpha_filter.h"
#include "analyzers/filters/empty_sentence_filter.h"
#include "corpus/corpus.h"
#include "util/shim.h"

namespace meta
{
namespace lm
{

template <size_t N>
language_model<N>::language_model(const std::string& config_file)
    : interp_{config_file}
{
    std::cout << "Creating " << N << "-gram language model" << std::endl;

    auto corpus = corpus::corpus::load(config_file);

    using namespace analyzers;
    std::unique_ptr<token_stream> stream;
    stream = make_unique<tokenizers::icu_tokenizer>();
    stream = make_unique<filters::lowercase_filter>(std::move(stream));
    stream = make_unique<filters::alpha_filter>(std::move(stream));
    stream = make_unique<filters::empty_sentence_filter>(std::move(stream));

    while (corpus->has_next())
    {
        auto doc = corpus->next();
        stream->set_content(doc.content());

        // get ngram stream started
        std::deque<std::string> ngram;
        for (size_t i = 1; i < N; ++i)
            ngram.push_back("<s>");

        // count each ngram occurrence
        while (*stream)
        {
            auto token = stream->next();
            if (N > 1)
            {
                ++dist_[make_string(ngram)][token];
                ngram.pop_front();
                ngram.push_back(token);
            }
            else
                ++dist_[""][token]; // unigram has no previous tokens
        }
    }

    // turn counts into probabilities
    for (auto& map : dist_)
    {
        double sum = 0.0;
        for (auto& end : map.second)
            sum += end.second;
        for (auto& end : map.second)
            end.second /= sum;
    }
}

template <size_t N>
std::string language_model
    <N>::next_token(const std::deque<std::string>& tokens, double random) const
{
    auto str = make_string(tokens);
    auto it = dist_.find(str);
    if (it == dist_.end())
        throw std::runtime_error{"couldn't find previous n - 1 tokens: " + str};

    double cur = 0.0;
    for (auto& end : it->second)
    {
        cur += end.second;
        if (cur > random)
            return end.first;
    }

    throw std::runtime_error{"could not generate next token: " + str};
}

template <size_t N>
std::string language_model<N>::generate(unsigned int seed) const
{
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> rdist(0.0, 1.0);

    // start generating at the beginning of a sequence
    std::deque<std::string> ngram;
    for (size_t n = 1; n < N; ++n)
        ngram.push_back("<s>");

    // keep generating until we see </s>
    std::string output;
    std::string next = next_token(ngram, rdist(gen));
    while (next != "</s>")
    {
        if (ngram.front() != "<s>")
            output += " " + ngram.front();
        ngram.pop_front();
        ngram.push_back(next);
        next = next_token(ngram, rdist(gen));
    }

    output += make_string(ngram);
    return output;
}

template <size_t N>
double language_model<N>::prob(std::deque<std::string> tokens) const
{
    std::deque<std::string> interp_tokens{tokens};
    interp_tokens.pop_back();
    auto interp_prob = interp_.prob(interp_tokens);

    auto last = tokens.back();
    tokens.pop_front();

    auto ngram = make_string(tokens);

    auto endings = dist_.find(ngram);
    if (endings == dist_.end())
        return (1.0 - lambda_) * interp_prob;

    auto prob = endings->second.find(last);
    if (prob == endings->second.end())
        return (1.0 - lambda_) * interp_prob;

    return lambda_ * prob->second + (1.0 - lambda_) * interp_prob;
}

template <size_t N>
double language_model<N>::perplexity(const std::string& tokens) const
{
    std::deque<std::string> ngram;
    for (size_t i = 1; i < N; ++i)
        ngram.push_back("<s>");

    double perp = 0.0;
    for (auto& token : make_deque(tokens))
    {
        ngram.push_back(token);
        double cur_prob = std::log(1.0 + prob(ngram));
        perp += cur_prob;
        ngram.pop_front();
    }

    return std::pow(perp, 1.0 / N);
}

template <size_t N>
double language_model<N>::perplexity_per_word(const std::string& tokens) const
{
    return perplexity(tokens) / tokens.size();
}

template <size_t N>
std::deque<std::string> language_model
    <N>::make_deque(const std::string& tokens) const
{
    std::deque<std::string> d;
    std::stringstream sstream{tokens};
    std::string token;
    while (sstream >> token)
        d.push_back(token);

    return d;
}

template <size_t N>
std::string language_model
    <N>::make_string(const std::deque<std::string>& tokens) const
{
    std::string result{""};
    if (tokens.empty())
        return result;

    for (auto& token : tokens)
        result += token + " ";

    return result.substr(0, result.size() - 1); // remove trailing space
}
}
}
