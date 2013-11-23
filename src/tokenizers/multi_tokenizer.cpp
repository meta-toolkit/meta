/**
 * @file multi_tokenizer.cpp
 */

#include "corpus/document.h"
#include "tokenizers/multi_tokenizer.h"

namespace meta {
namespace tokenizers {

multi_tokenizer::multi_tokenizer(
        const std::vector<std::shared_ptr<tokenizer>> & toks):
    _tokenizers{toks}
{ /* nothing */ }

void multi_tokenizer::tokenize(corpus::document & doc)
{
    for(auto & tok: _tokenizers)
        tok->tokenize(doc);
}

}
}
