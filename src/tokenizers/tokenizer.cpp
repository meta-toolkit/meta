/**
 * @file tokenizer.cpp
 */

#include <fstream>
#include <iostream>
#include "util/invertible_map.h"
#include "tokenizers/parse_tree.h"
#include "tokenizers/tokenizer.h"

namespace meta {
namespace tokenizers {

using std::string;
using std::cout;
using std::endl;
using std::ofstream;
using std::unordered_map;

using util::InvertibleMap;
using index::TermID;
using index::Document;

Tokenizer::Tokenizer():
    _termMap(InvertibleMap<TermID, string>()),
    _currentTermID(0)
{ /* nothing */ }

TermID Tokenizer::getMapping(const string & term)
{
    if(!_termMap.containsValue(term))
    {
        _termMap.insert(_currentTermID, term);
        return _currentTermID++;
    }
    else
    {
        TermID termID = _termMap.getKeyByValue(term);
        return termID;
    }
}

void Tokenizer::setTermIDMapping(const InvertibleMap<TermID, string> & mapping)
{
    _termMap = mapping;
    _currentTermID = _termMap.size();
}

void Tokenizer::saveTermIDMapping(const string & filename) const
{
    _termMap.saveMap(filename);
}

const InvertibleMap<TermID, std::string> & Tokenizer::getTermIDMapping() const
{
    return _termMap;
}

string Tokenizer::getLabel(TermID termID) const
{
    return _termMap.getValueByKey(termID);
}

void Tokenizer::printData() const
{
    for(auto & term: _termMap)
        cout << term.first << "\t" << term.second << endl;
}

void Tokenizer::setMaxTermID(size_t start)
{
    _currentTermID = start;
}

TermID Tokenizer::getMaxTermID() const
{
    return _currentTermID;
}

size_t Tokenizer::getNumTerms() const
{
    return _termMap.size();

}

}
}
