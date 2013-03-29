/**
 * @file multi_tokenizer.cpp
 */

#include "index/document.h"
#include "tokenizers/multi_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::unordered_map;
using std::string;
        
MultiTokenizer::MultiTokenizer(const std::vector<std::shared_ptr<Tokenizer>> & toks):
    _tokenizers(toks),
    _maxTermID(0)
{ /* nothing */ }

void MultiTokenizer::tokenize(index::Document & document,
    const std::shared_ptr<unordered_map<index::TermID, unsigned int>> & docFreq)
{
    for(auto & tok: _tokenizers)
    {
        // adjust the other tokenizer's max termID so the features don't get
        // out of sync (every feature needs a unique ID, regardless of the
        // tokenizer that creates it)
        tok->setMaxTermID(_maxTermID);

        // tokenize the current document
        tok->tokenize(document, docFreq);

        // update the number of unique terms
        _maxTermID = tok->getMaxTermID();

        // update the counts stored in our own _termMap
        for(auto & m: tok->getTermIDMapping())
            _termMap.insert(m.first, m.second);
    }
}

}
}
