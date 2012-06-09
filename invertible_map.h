/**
 * @file invertible_map.h
 */

#ifndef _INVERTIBLE_MAP_H_
#define _INVERTIBLE_MAP_H_

#include <iostream>
#include <map>
#include <unordered_map>

using std::map;
using std::unordered_map;
using std::cerr;
using std::endl;

/**
 * This data structure indexes by keys as well as values, allowing constant
 *  amortized lookup time by key or value. All keys and values must be unique.
 */
template <class Key, class Value>
class InvertibleMap
{
    InvertibleMap();
    ~InvertibleMap();
    const InvertibleMap & operator=(const InvertibleMap & other);

    bool empty() const;
    size_t size() const;

    Key getKeyByValue(const Key & key) const;
    Value getValueByKey(const Value & value) const;

    void insert(const Key & key, const Value & value);

    map<Key, Value> sortKeys() const;
    map<Value, Key> sortValues() const;
};

#include "invertible_map.cpp"
#endif
