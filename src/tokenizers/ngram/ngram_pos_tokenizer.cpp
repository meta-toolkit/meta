/**
 * @file ngram_pos_tokenizer.cpp
 */

#include <unordered_map>
#include "io/parser.h"
#include "tokenizers/ngram/ngram_pos_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_pos_tokenizer::ngram_pos_tokenizer(uint16_t n):
    ngram_simple_tokenizer{n}
{ /* nothing */ }

void ngram_pos_tokenizer::tokenize(corpus::document & doc)
{
    io::parser parser{create_parser(doc, ".pos", " \n")};
    simple_tokenize(parser, doc);
}

}
}
