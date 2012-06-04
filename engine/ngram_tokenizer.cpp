#include "ngram_tokenizer.h"

void NgramTokenizer::tokenize(const string & filename, Document & document) const
{
    unordered_map<string, size_t> tokens;
    return;
}

size_t NgramTokenizer::getNValue() const
{
    return _nValue;
}
