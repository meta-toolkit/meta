/**
 * @file invertible_map.cpp
 */

#include "io/parser.h"
#include "util/common.h"

using std::istringstream;
using std::ofstream;
using std::pair;
using std::make_pair;
using std::map;
using std::unordered_map;
using std::cerr;
using std::endl;

template <class Key, class Value>
InvertibleMap<Key, Value>::InvertibleMap():
    _forward(unordered_map<Key, Value>()),
    _backward(unordered_map<Value, Key>()) { /* nothing */ }

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
map<Key, Value> InvertibleMap<Key, Value>::sortKeys() const
{
    map<Key, Value> sorted;
    for(auto & it: _forward)
        sorted.insert(it);
    return sorted;
}

template <class Key, class Value>
map<Value, Key> InvertibleMap<Key, Value>::sortValues() const
{
    map<Value, Key> sorted;
    for(auto & it: _backward)
        sorted.insert(it);
    return sorted;
}

template <class Key, class Value>
void InvertibleMap<Key, Value>::saveMap(const string & filename) const
{
    ofstream outfile(filename);
    if(outfile.good())
    {
        for(auto & entry: _forward)
            outfile << entry.first << " " << entry.second << endl;
        outfile.close();
    }
    else
    {
        cerr << "[InvertibleMap]: error writing map to disk" << endl;
    }
}

template <class Key, class Value>
void InvertibleMap<Key, Value>::readMap(const string & filename)
{
    Parser parser(filename, " \n");
    while(parser.hasNext())
    {
        Key key;
        Value value;
        istringstream(parser.next()) >> key;
        istringstream(parser.next()) >> value;
        insert(key, value);
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
