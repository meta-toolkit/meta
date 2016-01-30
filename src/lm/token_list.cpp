/**
 * @file token_list.cpp
 * @author Sean Massung
 */

#include "meta/lm/token_list.h"

namespace meta
{
namespace lm
{
token_list::token_list(const std::string& ngram,
           const std::unordered_map<std::string, term_id>& vocab)
{
    std::string token;
    std::istringstream iss{ngram};
    while (iss >> token)
    {
        auto it = vocab.find(token);
        if (it != vocab.end())
            tokens_.push_back(it->second);
        else
            tokens_.push_back(vocab.at("<unk>"));
    }
}

token_list::token_list(const lm::sentence& ngram,
           const std::unordered_map<std::string, term_id>& vocab)
{
    tokens_.reserve(ngram.size());
    for (const auto& token : ngram)
    {
        auto it = vocab.find(token);
        if (it != vocab.end())
            tokens_.push_back(it->second);
        else
            tokens_.push_back(vocab.at("<unk>"));
    }
}

token_list::token_list(term_id val)
{
    tokens_.push_back(val);
}

const term_id& token_list::operator[](uint64_t idx) const
{
    return tokens_[idx];
}

term_id& token_list::operator[](uint64_t idx)
{
    return tokens_[idx];
}

uint64_t token_list::size() const
{
    return tokens_.size();
}

void token_list::push_back(term_id elem)
{
    tokens_.push_back(elem);
}

void token_list::pop_front()
{
    tokens_.erase(tokens_.begin());
}

void token_list::pop_back()
{
    tokens_.pop_back();
}

const std::vector<term_id>& token_list::tokens() const
{
    return tokens_;
}
}
}
