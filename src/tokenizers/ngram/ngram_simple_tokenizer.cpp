/**
 * @file ngram_simple_tokenizer.cpp
 */

#include "tokenizers/ngram/ngram_simple_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_simple_tokenizer::ngram_simple_tokenizer(uint16_t n):
    ngram_tokenizer{n}
{ /* nothing */ }

void ngram_simple_tokenizer::simple_tokenize(io::parser & parser,
        corpus::document & doc)
{
    // initialize the ngram
    std::deque<std::string> ngram;
    for(size_t i = 0; i < n_value() && parser.has_next(); ++i)
        ngram.push_back(parser.next());

    // add the rest of the ngrams
    while(parser.has_next())
    {
        doc.increment(wordify(ngram), 1);
        ngram.pop_front();
        ngram.push_back(parser.next());
    }

    // add the last token
    doc.increment(wordify(ngram), 1);
}

}
}
