#include "document.h" 

Document::Document(string name, string category):
    _frequencies(unordered_map<string, size_t>()),
    _name(name),
    _category(category),
    _length(0)
{ /* nothing */ }

void Document::increment(string transition, size_t amount, unordered_map<string, size_t>* docFreq)
{
    unordered_map<string, size_t>::iterator iter;
    iter = _frequencies.find(transition);
   
    if(iter != _frequencies.end())
        iter->second += amount;
    else
    {
        _frequencies.insert(make_pair(transition, amount));
        if(docFreq)
            (*docFreq)[transition]++;
    }

    _length += amount;
   
}

void Document::increment(string transition, size_t amount)
{
    increment(transition, amount, NULL);
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

size_t Document::getFrequency(string transition) const
{
    unordered_map<string, size_t>::const_iterator iter;
    iter = _frequencies.find(transition);
    if(iter != _frequencies.end())
        return iter->second;
    else
        return 0;
}

const unordered_map<string, size_t> & Document::getFrequencies() const
{
    return _frequencies;
}
