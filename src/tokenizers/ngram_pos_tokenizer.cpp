/**
 * @file ngram_pos_tokenizer.cpp
 */

#include <unordered_map>
#include "io/parser.h"
#include "tokenizers/ngram_pos_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_pos_tokenizer::ngram_pos_tokenizer(size_t n):
    ngram_simple_tokenizer(n) { /* nothing */ }

void ngram_pos_tokenizer::tokenize_document(index::Document & document,
        std::function<term_id(const std::string &)> mapping,
        const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq)
{
    io::Parser parser(document.getPath() + ".pos", " \n");
    simple_tokenize(parser, document, mapping, docFreq);
}

}
}
