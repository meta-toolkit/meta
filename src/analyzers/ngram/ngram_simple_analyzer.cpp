/**
 * @file ngram_simple_analyzer.cpp
 */

#include "corpus/document.h"
#include "analyzers/ngram/ngram_simple_analyzer.h"

namespace meta {
namespace analyzers {

ngram_simple_analyzer::ngram_simple_analyzer(uint16_t n):
    ngram_analyzer{n}
{ /* nothing */ }

void ngram_simple_analyzer::simple_tokenize(io::parser & parser,
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
