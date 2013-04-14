/**
 * @file multi_tokenizer.cpp
 */

#include "index/document.h"
#include "tokenizers/multi_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::unordered_map;
        
multi_tokenizer::multi_tokenizer(const std::vector<std::shared_ptr<tokenizer>> & toks):
    _tokenizers(toks),
    _max_term_id(0)
{ /* nothing */ }

void multi_tokenizer::tokenize(index::Document & document,
    const std::shared_ptr<unordered_map<term_id, unsigned int>> & docFreq)
{
    for(auto & tok: _tokenizers)
    {
        // adjust the other tokenizer's max termID so the features don't get
        // out of sync (every feature needs a unique ID, regardless of the
        // tokenizer that creates it)
        tok->set_max_term_id(_max_term_id);

        // tokenize the current document
        tok->tokenize(document, docFreq);

        // update the number of unique terms
        _max_term_id = tok->max_term_id();

        // update the counts stored in our own _termMap
        for(auto & m: tok->term_id_mapping())
            _term_map.insert(m.first, m.second);
    }
}

}
}
