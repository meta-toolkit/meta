/**
 * @file document.cpp
 */

#include "document.h" 

Document::Document(string name, string category):
    _frequencies(unordered_map<TermID, unsigned int>()),
    _name(name),
    _category(category),
    _length(0)
{ /* nothing */ }

void Document::increment(TermID termID, unsigned int amount, unordered_map<TermID, unsigned int>* docFreq)
{
    unordered_map<TermID, unsigned int>::iterator iter;
    iter = _frequencies.find(termID);
   
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

string Document::getName() const
{
    return _name;
}

string Document::getCategory() const
{
    return _category;
}

size_t Document::getLength() const
{
    return _length;
}

size_t Document::getFrequency(TermID termID) const
{
    unordered_map<TermID, unsigned int>::const_iterator iter;
    iter = _frequencies.find(termID);
    if(iter != _frequencies.end())
        return iter->second;
    else
        return 0;
}

const unordered_map<TermID, unsigned int> & Document::getFrequencies() const
{
    return _frequencies;
}
