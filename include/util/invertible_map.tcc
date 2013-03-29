/**
 * @file invertible_map.tcc
 */

#include "io/parser.h"
#include "util/common.h"

namespace meta {
namespace util {

template <class Key, class Value>
InvertibleMap<Key, Value>::InvertibleMap():
    _forward(std::unordered_map<Key, Value>()),
    _backward(std::unordered_map<Value, Key>()) { /* nothing */ }

template <class Key, class Value>
bool InvertibleMap<Key, Value>::empty() const
{
    return _forward.empty();
}

template <class Key, class Value>
size_t InvertibleMap<Key, Value>::size() const
{
    return _forward.size();
}

template <class Key, class Value>
Key InvertibleMap<Key, Value>::getKeyByValue(const Value & value) const
{
    auto it = _backward.find(value);
    if(it == _backward.end())
        return Key();
    return it->second;
}

template <class Key, class Value>
Value InvertibleMap<Key, Value>::getValueByKey(const Key & key) const
{
    auto it = _forward.find(key);
    if(it == _forward.end())
        return Value();
    return it->second;
}

template <class Key, class Value>
bool InvertibleMap<Key, Value>::containsKey(const Key & key) const
{
    return _forward.find(key) != _forward.end();
}

template <class Key, class Value>
bool InvertibleMap<Key, Value>::containsValue(const Value & value) const
{
    return _backward.find(value) != _backward.end();
}

template <class Key, class Value>
void InvertibleMap<Key, Value>::insert(const Key & key, const Value & value)
{
    _forward.insert(make_pair(key, value));
    _backward.insert(make_pair(value, key));
}

template <class Key, class Value>
std::map<Key, Value> InvertibleMap<Key, Value>::sortKeys() const
{
    std::map<Key, Value> sorted;
    for(auto & it: _forward)
        sorted.insert(it);
    return sorted;
}

template <class Key, class Value>
std::map<Value, Key> InvertibleMap<Key, Value>::sortValues() const
{
    std::map<Value, Key> sorted;
    for(auto & it: _backward)
        sorted.insert(it);
    return sorted;
}

template <class Key, class Value>
void InvertibleMap<Key, Value>::saveMap(const std::string & filename) const
{
    std::ofstream outfile(filename);
    if(outfile.good())
    {
        for(auto & entry: _forward)
            outfile << entry.first << " " << entry.second << std::endl;
        outfile.close();
    }
    else
        throw invertible_map_exception("error writing map to disk");
}

template <class Key, class Value>
void InvertibleMap<Key, Value>::readMap(const std::string & filename)
{
    std::ifstream infile( filename );
    while( infile ) {
        std::string line;
        std::getline( infile, line );
        if( line.length() == 0 )
            continue;
        std::stringstream stream( line );
        Key key;
        Value value;
        stream >> key;
        stream >> value;
        insert( key, value );
    }
}

/* Iterator code */

template <class Key, class Value>
InvertibleMap<Key, Value>::Iterator::Iterator(): iter(InnerIterator())
    { /* nothing */ }

template <class Key, class Value>
InvertibleMap<Key, Value>::Iterator::Iterator(const InnerIterator & other): iter(other)
    { /* nothing */ }

template <class Key, class Value>
typename InvertibleMap<Key, Value>::Iterator & InvertibleMap<Key, Value>::Iterator::operator++()
{
    ++iter;
    return *this;
}

template <class Key, class Value>
typename InvertibleMap<Key, Value>::Iterator InvertibleMap<Key, Value>::Iterator::operator++(int)
{
    InnerIterator save = iter;
    ++iter;
    return Iterator(save);
}

template <class Key, class Value>
typename InvertibleMap<Key, Value>::Iterator & InvertibleMap<Key, Value>::Iterator::operator--()
{
    --iter;
    return *this;
}

template <class Key, class Value>
typename InvertibleMap<Key, Value>::Iterator InvertibleMap<Key, Value>::Iterator::operator--(int)
{
    InnerIterator save = iter;
    --iter;
    return Iterator(save);
}

template <class Key, class Value>
bool InvertibleMap<Key, Value>::Iterator::operator==(const Iterator & other)
{
    return iter == other.iter;
}

template <class Key, class Value>
bool InvertibleMap<Key, Value>::Iterator::operator!=(const Iterator & other)
{
    return iter != other.iter;
}

template <class Key, class Value>
const typename InvertibleMap<Key, Value>::InnerIterator::value_type & InvertibleMap<Key, Value>::Iterator::operator*()
{
    return *iter;
}

template <class Key, class Value>
const typename InvertibleMap<Key, Value>::InnerIterator::value_type* InvertibleMap<Key, Value>::Iterator::operator->()
{
    return &(*iter);
}

template <class Key, class Value>
typename InvertibleMap<Key, Value>::const_iterator InvertibleMap<Key, Value>::begin() const
{
    return Iterator(_forward.begin());
}

template <class Key, class Value>
typename InvertibleMap<Key, Value>::const_iterator InvertibleMap<Key, Value>::end() const
{
    return Iterator(_forward.end());
}

}
}
