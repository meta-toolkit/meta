#include "document.h" 

Document::Document(string author, string nationality):
    _frequencies(unordered_map<string, size_t>()),
    _author(author),
    _nationality(nationality)
{ /* nothing */ }

void Document::increment(string transition, size_t amount)
{
    unordered_map<string, size_t>::iterator iter;
    iter = _frequencies.find(transition);
   
    if(iter != _frequencies.end())
        iter->second += amount;
    else
        _frequencies.insert(make_pair(transition, amount));    

    _length += amount;
}

string Document::getAuthor() const
{
    return _author;
}

string Document::getNationality() const
{
    return _nationality;
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
