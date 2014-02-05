/**
 * @file ngram_tokenizer.cpp
 * @author Sean Massung
 */

#include "tokenizers/ngram/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_tokenizer::ngram_tokenizer(uint16_t n):
    _n_val{n}
{ /* nothing */ }

uint16_t ngram_tokenizer::n_value() const
{
    return _n_val;
}

std::string ngram_tokenizer::wordify(
        const std::deque<std::string> & words) const
{
    std::string result = "";
    for(auto & word: words)
        result += (word + "_");
    return result.substr(0, result.size() - 1);
}

}
}
