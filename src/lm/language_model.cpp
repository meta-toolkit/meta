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
    auto group = config.get_group("language-model");
    auto nval = group->get_as<int64_t>("n-value");
    if (!nval)
        throw language_model_exception{
            "no n-value specified in language-model group"};

    N_ = *nval;

    if (N_ > 1)
        interp_ = make_unique<language_model>(config_file, N_ - 1);

    select_method(config_file);
}

void language_model::select_method(const std::string& config_file)
{
    std::cout << "Creating " << N_ << "-gram language model" << std::endl;

    auto config = cpptoml::parse_file(config_file);
    auto group = config.get_group("language-model");
    auto format = group->get_as<std::string>("format");
    if (!format)
        throw language_model_exception{
            "no format specified in language-model group"};

    if (*format == "precomputed")
    {
        auto prefix = group->get_as<std::string>("prefix");
        if (!prefix)
            throw language_model_exception{
                "no prefix specified for precomputed language model"};
        read_precomputed(*prefix);
    }
    else if (*format == "learn")
        learn_model(config_file);
    else
        throw language_model_exception{
            "language-model format could not be determined"};
}

language_model::language_model(const std::string& config_file, size_t n) : N_{n}
{
    if (N_ > 1)
        interp_ = make_unique<language_model>(config_file, N_ - 1);

    select_method(config_file);
}

void language_model::learn_model(const std::string& config_file)
{
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
        sentence ngram;
        for (size_t i = 1; i < N_; ++i)
            ngram.push_back("<s>");

        // count each ngram occurrence
        while (*stream)
        {
            auto token = stream->next();
            if (N_ > 1)
            {
                ++dist_[ngram.to_string()][token];
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

void language_model::read_precomputed(const std::string& prefix)
{
    std::ifstream in{prefix + std::to_string(N_) + "-grams.txt"};
    std::string line;
    uint64_t count;
    while (in)
    {
        std::getline(in, line);
        std::istringstream iss{line};
        iss >> count;
        sentence ngram;
        std::string token;
        for (size_t i = 0; i < N_ - 1; ++i)
        {
            iss >> token;
            ngram.push_back(token);
        }

        // if there is one remaining token to read
        if (iss)
        {
            iss >> token;
            dist_[ngram.to_string()][token] = count;
        }
        else // if unigram
        {
            dist_[""][ngram.to_string()] = count;
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

std::string language_model::next_token(const sentence& tokens,
                                       double random) const
{
    auto it = dist_.find(tokens.to_string());
    if (it == dist_.end())
        throw language_model_exception{"couldn't find previous n - 1 tokens: "
                                       + tokens.to_string()};

    double cur = 0.0;
    for (auto& end : it->second)
    {
        cur += end.second;
        if (cur > random)
            return end.first;
    }

    throw language_model_exception{"could not generate next token: "
                                   + tokens.to_string()};
}

std::vector<std::pair<std::string, double>>
    language_model::top_k(const sentence& prev, size_t k) const
{
    if (prev.size() != N_ - 1)
        throw language_model_exception{"prev should contain n - 1 tokens"};

    auto it = dist_.find(prev.to_string());
    if (it == dist_.end())
        throw language_model_exception{"no transitions found"};

    using pair_t = std::pair<std::string, double>;
    std::vector<pair_t> probs{it->second.begin(), it->second.end()};

    auto comp = [&](const pair_t& a, const pair_t& b)
    {
        return a.second > b.second;
    };
    if (k >= probs.size())
    {
        std::sort(probs.begin(), probs.end(), comp);
        return probs;
    }

    std::nth_element(probs.begin(), probs.begin() + k, probs.end(), comp);
    std::vector<pair_t> sorted{probs.begin(), probs.begin() + k};
    std::sort(sorted.begin(), sorted.end(), comp);

    return sorted;
}

std::string language_model::generate(unsigned int seed) const
{
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> rdist(0.0, 1.0);

    // start generating at the beginning of a sequence
    sentence ngram;
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

    output += ngram.to_string();
    return output;
}

double language_model::prob(sentence tokens) const
{
    if (tokens.size() != N_)
        throw language_model_exception{"prob() needs one N-gram"};

    sentence interp_tokens{tokens};
    interp_tokens.pop_front(); // look at prev N - 1
    auto interp_prob = interp_ ? interp_->prob(interp_tokens) : 1.0;

    auto last = tokens.back();
    tokens.pop_back();

    auto endings = dist_.find(tokens.to_string());
    if (endings == dist_.end())
        return (1.0 - lambda_) * interp_prob;

    auto prob = endings->second.find(last);
    if (prob == endings->second.end())
        return (1.0 - lambda_) * interp_prob;

    return lambda_ * prob->second + (1.0 - lambda_) * interp_prob;
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
    return perplexity(tokens) / tokens.size();
}
}
}
