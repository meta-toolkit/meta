/**
 * @file ngram_simple_tokenizer.cpp
 */

#include "tokenizers/ngram/ngram_simple_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::deque;
using std::string;
using std::unordered_map;
using std::unordered_set;
using index::Document;
using io::Parser;

ngram_simple_tokenizer::ngram_simple_tokenizer(size_t n):
    ngram_tokenizer(n) { /* nothing */ }

void ngram_simple_tokenizer::simple_tokenize(Parser & parser, Document & document,
        std::function<term_id(const std::string &)> mapping,
        const std::shared_ptr<unordered_map<term_id, unsigned int>> & docFreq)
{
    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < n_value() && parser.hasNext(); ++i)
        ngram.push_back(parser.next());

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(mapping(wordified), 1, docFreq);
        ngram.pop_front();
        ngram.push_back(parser.next());
    }

    // add the last token
    document.increment(mapping(wordify(ngram)), 1, docFreq);
}

}
}
