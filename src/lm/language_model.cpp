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
#include "cpptoml.h"
#include "analyzers/analyzer.h"
#include "analyzers/tokenizers/icu_tokenizer.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/filters/alpha_filter.h"
#include "analyzers/filters/empty_sentence_filter.h"
#include "corpus/corpus.h"
#include "util/shim.h"
#include "lm/language_model.h"

namespace meta
{
namespace lm
{

language_model::language_model(const std::string& config_file)
{
    auto config = cpptoml::parse_file(config_file);
    auto group = config.get_table("language-model");
    auto nval = group->get_as<int64_t>("n-value");
    if(!nval)
        throw std::runtime_error{"no n-value specified in language-model group"};

    N_ = *nval;

    if (N_ > 1)
        interp_ = make_unique<language_model>(config_file, N_ - 1);

    learn_model(config_file);
}

language_model::language_model(const std::string& config_file, size_t n):
    N_{n}
{
    if (N_ > 1)
        interp_ = make_unique<language_model>(config_file, N_ - 1);

    learn_model(config_file);
}

void language_model::learn_model(const std::string& config_file)
{
    std::cout << "Creating " << N_ << "-gram language model" << std::endl;

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
        for (size_t i = 1; i < N_; ++i)
            ngram.push_back("<s>");

        // count each ngram occurrence
        while (*stream)
        {
            auto token = stream->next();
            if (N_ > 1)
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

std::string language_model::next_token(const std::deque<std::string>& tokens,
                                       double random) const
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

std::string language_model::generate(unsigned int seed) const
{
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> rdist(0.0, 1.0);

    // start generating at the beginning of a sequence
    std::deque<std::string> ngram;
    for (size_t n = 1; n < N_; ++n)
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

double language_model::prob(std::deque<std::string> tokens) const
{
    if (tokens.size() != N_)
        throw std::runtime_error{"prob() needs one N-gram"};

    std::deque<std::string> interp_tokens{tokens};
    interp_tokens.pop_front(); // look at prev N - 1
    auto interp_prob = interp_ ? interp_->prob(interp_tokens) : 1.0;

    auto last = tokens.back();
    tokens.pop_back();

    auto ngram = make_string(tokens);

    auto endings = dist_.find(ngram);
    if (endings == dist_.end())
        return (1.0 - lambda_) * interp_prob;

    auto prob = endings->second.find(last);
    if (prob == endings->second.end())
        return (1.0 - lambda_) * interp_prob;

    return lambda_ * prob->second + (1.0 - lambda_) * interp_prob;
}

double language_model::perplexity(const std::string& tokens) const
{
    std::deque<std::string> ngram;
    for (size_t i = 1; i < N_; ++i)
        ngram.push_back("<s>");

    double perp = 0.0;
    for (auto& token : make_deque(tokens))
    {
        ngram.push_back(token);
        perp += std::log(1.0 + 1.0 / prob(ngram));
        ngram.pop_front();
    }

    return std::pow(perp, 1.0 / N_);
}

double language_model::perplexity_per_word(const std::string& tokens) const
{
    return perplexity(tokens) / tokens.size();
}

std::deque<std::string> language_model::make_deque(const std::string
                                                   & tokens) const
{
    std::deque<std::string> d;
    std::stringstream sstream{tokens};
    std::string token;
    while (sstream >> token)
        d.push_back(token);

    return d;
}

std::string language_model::make_string(const std::deque
                                        <std::string>& tokens) const
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
