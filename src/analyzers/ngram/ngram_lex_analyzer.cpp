/**
 * @file ngram_lex_analyzer.cpp
 */

#include "analyzers/ngram/ngram_lex_analyzer.h"

namespace meta {
namespace analyzers {

ngram_lex_analyzer::ngram_lex_analyzer(uint16_t n):
    ngram_simple_analyzer{n}
{ /* nothing */ }

void ngram_lex_analyzer::tokenize(corpus::document & doc)
{
    io::parser parser{create_parser(doc, ".lex", " \n")};
    simple_tokenize(parser, doc);
}

}
}
