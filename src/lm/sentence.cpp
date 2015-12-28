/**
 * @file sentence.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <numeric>
#include <iterator>
#include "meta/lm/sentence.h"
#include "meta/utf/segmenter.h"

namespace meta
{
namespace lm
{
sentence::sentence(const std::string& text, bool tokenize /* = true */)
{
    if (tokenize)
    {
        utf::segmenter segmenter;
        segmenter.set_content(text);
        for (const auto& word : segmenter.words())
        {
            auto str = segmenter.content(word);
            if (!str.empty() && !std::all_of(str.begin(), str.end(), ::isspace))
                tokens_.emplace_back(std::move(str));
        }

        if (tokens_.empty())
            throw sentence_exception{"empty token stream"};
    }
    else
    {
        std::istringstream iss{text};
        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(tokens_));
    }
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
    using diff_type = iterator::difference_type;
    ret.tokens_.insert(ret.tokens_.begin(),
                       tokens_.begin() + static_cast<diff_type>(from),
                       tokens_.begin() + static_cast<diff_type>(to));
    return ret;
}

void sentence::substitute(size_type idx, const std::string& token,
                          double weight /* = 0.0 */)
{
    ops_.push_back("substitute(" + tokens_[idx] + " -> " + token + ")");
    tokens_[idx] = token;
    weights_.push_back(weight);
}

void sentence::remove(size_type idx, double weight /* = 0.0 */)
{
    ops_.push_back("remove(" + (*this)[idx] + ")");
    using diff_type = iterator::difference_type;
    tokens_.erase(tokens_.begin() + static_cast<diff_type>(idx));
    weights_.push_back(weight);
}

void sentence::insert(size_type idx, const std::string& token,
                      double weight /* = 0.0 */)
{
    using diff_type = iterator::difference_type;
    tokens_.insert(tokens_.begin() + static_cast<diff_type>(idx), token);
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

std::vector<double> sentence::weights() const
{
    return weights_;
}

const std::vector<std::string>& sentence::operations() const
{
    return ops_;
}

const std::vector<std::string>& sentence::tokens() const
{
    return tokens_;
}

const std::string& sentence::front() const
{
    return tokens_.front();
}

const std::string& sentence::back() const
{
    return tokens_.back();
}

void sentence::push_front(const std::string& token)
{
    // we use a std::vector instead of a std::deque because sentence's
    // operator== (using std::vector) is significantly faster; push_front is not
    // called as much as operator==
    tokens_.insert(tokens_.begin(), token);
}

void sentence::pop_front()
{
    // we use a std::vector instead of a std::deque because sentence's
    // operator== (using std::vector) is significantly faster; pop_front is not
    // called as much as operator==
    tokens_.erase(tokens_.begin());
}

void sentence::push_back(const std::string& token)
{
    tokens_.push_back(token);
}

void sentence::pop_back()
{
    tokens_.pop_back();
}

template <class... Args>
void sentence::emplace_back(Args&&... args)
{
    tokens_.emplace_back(std::forward<Args>(args)...);
}

sentence::iterator sentence::begin()
{
    return tokens_.begin();
}

sentence::iterator sentence::end()
{
    return tokens_.end();
}

sentence::const_iterator sentence::begin() const
{
    return tokens_.cbegin();
}

sentence::const_iterator sentence::end() const
{
    return tokens_.cend();
}

sentence::size_type sentence::size() const
{
    return tokens_.size();
}
}
}
