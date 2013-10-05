/**
 * @file multi_tokenizer.cpp
 */

#include "corpus/document.h"
#include "tokenizers/multi_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::unordered_map;
        
multi_tokenizer::multi_tokenizer(const std::vector<std::shared_ptr<tokenizer>> & toks):
    _tokenizers(toks)
{ /* nothing */ }

void multi_tokenizer::tokenize_document(corpus::document & document,
    std::function<term_id(const std::string &)> mapping)
{
    for(auto & tok: _tokenizers)
        tok->tokenize_document(document, mapping);
}

}
}
