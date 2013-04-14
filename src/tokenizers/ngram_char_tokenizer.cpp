/**
 * @file ngram_char_tokenizer.cpp
 */

#include <unordered_map>
#include "io/parser.h"
#include "tokenizers/ngram_char_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_char_tokenizer::ngram_char_tokenizer(size_t n):
    ngram_simple_tokenizer(n) { /* nothing */ }

void ngram_char_tokenizer::tokenize(index::Document & document,
        const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq)
{
    io::Parser parser(document.getPath() + ".sen", " \n");
    simple_tokenize(parser, document, docFreq);
}

}
}
