#include <fstream>
#include <omp.h>
#include "util/invertible_map.h"
#include "index/document.h"
#include "parse_tree.h"
#include "tokenizer.h"

using std::ofstream;
using std::unordered_map;

Tokenizer::Tokenizer():
    _currentTermID(0),
    _termMap(InvertibleMap<TermID, string>())
{ /* nothing */ }

TermID Tokenizer::getMapping(const string & term)
{
    TermID retID;
    #pragma omp critical
    {
        if(!_termMap.containsValue(term))
        {
            _termMap.insert(_currentTermID, term);
            retID = _currentTermID++;
        }
        else
        {
            TermID termID = _termMap.getKeyByValue(term);
            retID = termID;
        }
    }
    return retID;
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

string Tokenizer::getLabel(TermID termID) const
{
    return _termMap.getValueByKey(termID);
}
