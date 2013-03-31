/**
 * @file ngram_tokenizer.cpp
 */

#include "tokenizers/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::deque;
using std::string;
using std::unordered_map;
using std::unordered_set;

using index::TermID;
using index::Document;
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

string ngram_tokenizer::set_lower(const string & original) const
{
    string word = "";
    for(auto ch: original)
    {
        if(ch >= 'A' && ch <= 'Z')
            ch += 32;
        word += ch;
    }
    return word;
}

}
}
