/**
 * @file multi_tokenizer.cpp
 */

#include "index/document.h"
#include "tokenizers/multi_tokenizer.h"

using std::unordered_map;
using std::string;
        
MultiTokenizer::MultiTokenizer(const std::vector<std::shared_ptr<Tokenizer>> & tokenizers):
    _tokenizers(tokenizers),
    _maxTermID(0)
{ /* nothing */ }

void MultiTokenizer::tokenize(Document & document,
    const std::shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
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
    }
}

string MultiTokenizer::getLabel(TermID termID) const
{
    for(auto & t: _tokenizers)
    {
        auto map = t->getTermIDMapping();
        if(map.containsKey(termID))
            return map.getValueByKey(termID);
    }

    return "";
}
