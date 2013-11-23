/**
 * @file ngram_lex_tokenizer.cpp
 */

#include <unordered_map>
#include "io/parser.h"
#include "tokenizers/ngram/ngram_lex_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_lex_tokenizer::ngram_lex_tokenizer(uint16_t n):
    ngram_simple_tokenizer{n}
{ /* nothing */ }

void ngram_lex_tokenizer::tokenize(corpus::document & doc)
{
    io::parser parser{create_parser(doc, ".lex", " \n")};
    simple_tokenize(parser, doc);
}

}
}
