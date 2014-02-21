/**
 * @file ngram_pos_analyzer.cpp
 */

#include "analyzers/ngram/ngram_pos_analyzer.h"

namespace meta {
namespace analyzers {

ngram_pos_analyzer::ngram_pos_analyzer(uint16_t n):
    base{n}
{ /* nothing */ }

void ngram_pos_analyzer::tokenize(corpus::document & doc)
{
    io::parser parser{create_parser(doc, ".pos", " \n")};
    simple_tokenize(parser, doc);
}

}
}
