/**
 * @file ngram_tokenizer.cpp
 */

#include "tokenizers/ngram/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::deque;
using std::string;
using std::unordered_map;
using std::unordered_set;
using index::document;
using io::Parser;

ngram_tokenizer::ngram_tokenizer(size_t n):
    _n_val(n) { /* nothing */ }

size_t ngram_tokenizer::n_value() const
{
    return _n_val;
}

string ngram_tokenizer::wordify(const deque<string> & words) const
{
    string result = "";
    for(auto & word: words)
        result += (word + " ");
    return result.substr(0, result.size() - 1);
}

}
}
