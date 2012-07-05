#include "tokenizer.h"

Tokenizer::Tokenizer():
    _currentTermID(0), _termMap(unordered_map<string, TermID>())
{ /* nothing */ }

TermID Tokenizer::getMapping(const string & term)
{
    auto it = _termMap.find(term);
    if(it == _termMap.end())
    {
        _termMap.insert(make_pair(term, _currentTermID));
        return _currentTermID++;
    }
    else
        return it->second;
}

void Tokenizer::tokenize(Document & document, unordered_map<TermID, unsigned int>* docFreq)
{

}

void Tokenizer::saveTermIDMapping(const string & filename) const
{
    ofstream outfile(filename);
    if(outfile.good())
    {
        for(auto & term: _termMap)
        {
            outfile << term.first << " ";
            outfile << term.second << endl;
        }
    }
    else
    {
        cerr << "[Tokenizer]: error creating termid mapping file" << endl;
    }
}
