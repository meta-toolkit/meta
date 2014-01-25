/**
 * @file ngram_char_tokenizer.cpp
 */

#include "corpus/document.h"
#include "io/parser.h"
#include "tokenizers/ngram/ngram_char_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_char_tokenizer::ngram_char_tokenizer(uint16_t n):
    ngram_simple_tokenizer{n}
{ /* nothing */ }

void ngram_char_tokenizer::tokenize(corpus::document & doc)
{
    io::parser parser(doc.path() + ".sen", " \n");
    simple_tokenize(parser, doc);
}

}
}
