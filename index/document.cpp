/**
 * @file document.cpp
 */

#include <utility>
#include "document.h" 

using std::pair;
using std::make_pair;
using std::string;
using std::unordered_map;

Document::Document(const string & path):
    _frequencies(unordered_map<TermID, unsigned int>()),
    _path(path),
    _length(0)
{
    _name = getName(path);
    _category = getCategory(path);
}

void Document::increment(TermID termID, unsigned int amount, unordered_map<TermID, unsigned int>* docFreq)
{
    auto iter = _frequencies.find(termID);
    if(iter != _frequencies.end())
        iter->second += amount;
    else
    {
        _frequencies.insert(make_pair(termID, amount));
        if(docFreq)
            (*docFreq)[termID]++;
    }

    _length += amount;
}

void Document::increment(TermID termID, unsigned int amount)
{
    increment(termID, amount, NULL);
}

string Document::getPath() const
{
    return _path;
}

string Document::getCategory() const
{
    return _category;
}

string Document::getName() const
{
    return _name;
}

size_t Document::getLength() const
{
    return _length;
}

size_t Document::getFrequency(TermID termID) const
{
    auto iter = _frequencies.find(termID);
    if(iter != _frequencies.end())
        return iter->second;
    else
        return 0;
}

const unordered_map<TermID, unsigned int> & Document::getFrequencies() const
{
    return _frequencies;
}

string Document::getName(const string & path)
{
    size_t idx = path.find_last_of("/") + 1;
    return path.substr(idx, path.size() - idx);
}

string Document::getCategory(const string & path)
{
    size_t idx = path.find_last_of("/");
    string sub = path.substr(0, idx);
    return getName(sub);
}
