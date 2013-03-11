/**
 * @file multi_tokenizer.cpp
 */

#include "tokenizers/multi_tokenizer.h"
using std::unordered_map;
        
MultiTokenizer::MultiTokenizer(const std::vector<Tokenizer*> & tokenizers):
    _tokenizers(tokenizers)
{ /* nothing */ }

MultiTokenizer::~MultiTokenizer()
{
    for(auto & tok: _tokenizers)
        delete tok;
}

void MultiTokenizer::tokenize(Document & document,
    std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    size_t maxTermID = 0;
    for(auto & tok: _tokenizers)
    {
        // adjust the other tokenizer's max termID so the features don't get
        // out of sync (every feature needs a unique ID, regardless of the
        // tokenizer that creates it)
        tok->setMaxTermID(maxTermID);

        // tokenize the current document
        tok->tokenize(document, docFreq);

        // update the number of unique terms
        maxTermID = tok->getNumTerms();
    }
}
