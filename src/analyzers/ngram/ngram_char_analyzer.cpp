/**
 * @file ngram_char_analyzer.cpp
 */

#include "corpus/document.h"
#include "io/parser.h"
#include "analyzers/ngram/ngram_char_analyzer.h"

namespace meta {
namespace analyzers {

ngram_char_analyzer::ngram_char_analyzer(uint16_t n):
    base{n}
{ /* nothing */ }

void ngram_char_analyzer::tokenize(corpus::document & doc)
{
    io::parser parser(doc.path() + ".sen", " \n");
    simple_tokenize(parser, doc);
}

}
}
