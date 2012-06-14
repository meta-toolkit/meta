/**
 * @file invertible_map.h
 */

#ifndef _INVERTIBLE_MAP_H_
#define _INVERTIBLE_MAP_H_

#include <utility>
#include <iostream>
#include <map>
#include <unordered_map>

using std::pair;
using std::make_pair;
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
    public:

        /**
         * Constructor.
         */
        InvertibleMap():
            _forward(unordered_map<Key, Value>()),
            _backward(unordered_map<Value, Key>()) { /* nothing */ }

        /**
         * @return whether the invertible map is empty
         */
        bool empty() const;

        /**
         * @return the number of elements in the invertible map
         */
        size_t size() const;

        /**
         * @return a key given a value
         */
        Key getKeyByValue(const Value & value) const;

        /**
         * @return a value given a key
         */
        Value getValueByKey(const Key & key) const;

        /**
         * Inserts a (key, value) pair into the invertible map
         */
        void insert(const Key & key, const Value & value);

        /**
         * @return a (key, value) map sorted by keys
         */
        map<Key, Value> sortKeys() const;

        /**
         * @return a (value, key) map sorted by values
         */
        map<Value, Key> sortValues() const;

    private:

        unordered_map<Key, Value> _forward;
        unordered_map<Value, Key> _backward;
};

#include "invertible_map.cpp"
#endif
