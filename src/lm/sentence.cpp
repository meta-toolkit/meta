/**
 * @file sentence.cpp
 * @author Sean Massung
 */

#include <iostream>

#include <numeric>
#include "lm/sentence.h"
#include "analyzers/analyzer.h"
#include "analyzers/tokenizers/icu_tokenizer.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/filters/blank_filter.h"
#include "analyzers/filters/empty_sentence_filter.h"

namespace meta
{
namespace lm
{
sentence::sentence(const std::string& text)
{
    using namespace analyzers;
    std::unique_ptr<token_stream> stream;
    stream = make_unique<tokenizers::icu_tokenizer>();
    stream = make_unique<filters::lowercase_filter>(std::move(stream));
    stream = make_unique<filters::blank_filter>(std::move(stream));
    stream = make_unique<filters::empty_sentence_filter>(std::move(stream));
    stream->set_content(text);
    while (*stream)
        tokens_.push_back(stream->next());

    if (tokens_.empty())
        throw sentence_exception{"empty token stream"};

    // remove sentence markers (they're inserted by the LM)
    tokens_.pop_front();
    tokens_.pop_back();

    if (tokens_.empty())
        throw sentence_exception{"empty token stream"};
}

std::string sentence::to_string() const
{
    std::string result{""};
    if (tokens_.empty())
        return result;

    for (auto& token : tokens_)
        result += token + " ";

    return result.substr(0, result.size() - 1); // remove trailing space
}

const std::string& sentence::operator[](size_type idx) const
{
    return tokens_[idx];
}

sentence sentence::operator()(size_type from, size_type to) const
{
    sentence ret;
    if (from > to || to > tokens_.size())
        throw sentence_exception{"operator() out of bounds: from = "
                                 + std::to_string(from) + ", to = "
                                 + std::to_string(to)};
    ret.tokens_.insert(ret.tokens_.begin(), tokens_.begin() + from,
                       tokens_.begin() + to);
    return ret;
}

void sentence::substitute(size_type idx, const std::string& token,
                          double weight /* = 0.0 */)
{
    //ops_.push_back("substitute(" + std::to_string(idx) + ", " + tokens_[idx]
    //               + " -> " + token + ")");
    ops_.push_back("substitute(" + tokens_[idx] + " -> " + token + ")");
    tokens_[idx] = token;
    weights_.push_back(weight);
}

void sentence::remove(size_type idx, double weight /* = 0.0 */)
{
    //ops_.push_back("remove(" + std::to_string(idx) + ", " + (*this)[idx] + ")");
    ops_.push_back("remove(" + (*this)[idx] + ")");
    tokens_.erase(tokens_.begin() + idx);
    weights_.push_back(weight);
}

void sentence::insert(size_type idx, const std::string& token,
                      double weight /* = 0.0 */)
{
    tokens_.insert(tokens_.begin() + idx, token);
    //ops_.push_back("insert(" + std::to_string(idx) + ", " + token + ")");
    ops_.push_back("insert(" + token + ")");
    weights_.push_back(weight);
}

double sentence::average_weight() const
{
    if (weights_.empty())
        return 0.0;
    double sum = std::accumulate(weights_.begin(), weights_.end(), 0.0);
    return sum / weights_.size();
}

std::vector<double> sentence::weights() const { return weights_; }

const std::vector<std::string>& sentence::operations() const { return ops_; }

std::string sentence::front() const { return tokens_.front(); }

std::string sentence::back() const { return tokens_.back(); }

void sentence::push_front(const std::string& token)
{
    tokens_.push_front(token);
}

void sentence::pop_front() { tokens_.pop_front(); }

void sentence::push_back(const std::string& token) { tokens_.push_back(token); }

void sentence::pop_back() { tokens_.pop_back(); }

sentence::iterator sentence::begin() { return tokens_.begin(); }

sentence::iterator sentence::end() { return tokens_.end(); }

sentence::const_iterator sentence::begin() const { return tokens_.cbegin(); }

sentence::const_iterator sentence::end() const { return tokens_.cend(); }

sentence::size_type sentence::size() const { return tokens_.size(); }
}
}
