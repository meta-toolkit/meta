#include "tokenizer.h"

Tokenizer::Tokenizer():
    _currentTermID(0), _termMap(unordered_map<string, TermID>())
{ /* nothing */ }

TermID Tokenizer::getMapping(const string & term)
{
    unordered_map<string, TermID>::iterator it = _termMap.find(term);
    if(it == _termMap.end())
    {
        _termMap.insert(make_pair(term, _currentTermID));
        return _currentTermID++;
    }
    else
        return it->second;
}

void Tokenizer::tokenize(const string & filename, Document & document, unordered_map<TermID, unsigned int>* docFreq)
{

}
