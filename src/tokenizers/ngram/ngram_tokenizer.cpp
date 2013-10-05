/**
 * @file ngram_tokenizer.cpp
 */

#include "tokenizers/ngram/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_tokenizer::ngram_tokenizer(size_t n):
    _n_val(n) { /* nothing */ }

size_t ngram_tokenizer::n_value() const
{
    return _n_val;
}

std::string ngram_tokenizer::wordify(
        const std::deque<std::string> & words) const
{
    std::string result = "";
    for(auto & word: words)
        result += (word + " ");
    return result.substr(0, result.size() - 1);
}

}
}
