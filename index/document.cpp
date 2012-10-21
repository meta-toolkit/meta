/**
 * @file document.cpp
 */

#include <map>
#include <utility>
#include <iostream>
#include "document.h" 

using std::map;
using std::cout;
using std::endl;
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

void Document::increment(TermID termID, unsigned int amount,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
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

void Document::printLiblinearData(InvertibleMap<string, int> & mapping) const
{
    cout << getMapping(mapping, getCategory(_path));
    map<TermID, unsigned int> sorted;

    // liblinear feature indices start at 1, not 0 like the tokenizers
    for(auto & freq: _frequencies)
        sorted.insert(make_pair(freq.first + 1, freq.second));

    for(auto & freq: sorted)
        cout << " " << freq.first << ":" << freq.second;

    cout << "\n";
}

int Document::getMapping(InvertibleMap<string, int> & mapping, const string & category)
{
    // see if entered already
    if(mapping.containsKey(category))
        return mapping.getValueByKey(category);

    // otherwise, inefficiently create new entry (starting at 1)
    int newVal = 1;
    while(mapping.containsValue(newVal))
        ++newVal;

    mapping.insert(category, newVal);
    return newVal;
}
