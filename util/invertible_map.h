/**
 * @file invertible_map.h
 */

#ifndef _INVERTIBLE_MAP_H_
#define _INVERTIBLE_MAP_H_

#include <fstream>
#include <iterator>
#include <utility>
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>

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
        InvertibleMap();

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
         * @return whether the map contains the given key
         */
        bool containsKey(const Key & key) const;

        /**
         * @return whether the map contains the given value
         */
        bool containsValue(const Value & value) const;

        /**
         * Inserts a (key, value) pair into the invertible map
         */
        void insert(const Key & key, const Value & value);

        /**
         * @return a (key, value) map sorted by keys
         */
        std::map<Key, Value> sortKeys() const;

        /**
         * @return a (value, key) map sorted by values
         */
        std::map<Value, Key> sortValues() const;

        /**
         * Saves an Invertible Map to disk.
         * @param filename - where to save the map
         * @param map - the map to save
         */
        void saveMap(const std::string & filename) const;

        /**
         * Reads a saved map from disk and loads it into the current InvertibleMap.
         * @param filename - the file where the map is saved
         */
        void readMap(const std::string & filename);

        /**
         * The "inner" iterator representation of the InvertibleMap.
         */
        typedef typename std::unordered_map<Key, Value>::const_iterator InnerIterator;

        /**
         * The InvertibleMap iterator is really just a wrapper for the forward (key -> value)
         *  unordered_map iterator. Use this iterator class the same way you'd use it on an
         *  unordered_map.
         */
        class Iterator: public std::iterator<std::bidirectional_iterator_tag, InnerIterator>
        {
            public:

                /** Constructor */
                Iterator();

                /** Copy constructor */
                Iterator(const InnerIterator & other);
                
                /** Pre-Increment */
                Iterator & operator++();

                /** Post-increment */
                Iterator operator++(int);

                /** Pre-decrement */
                Iterator & operator--();

                /** Post-decrement */
                Iterator operator--(int);

                /** Iterator equality */
                bool operator==(const Iterator & other);

                /** Iterator inequality */
                bool operator!=(const Iterator & other);

                /**
                 * Dereference operator. Returns the underlying value_type,
                 *  which will always be a std::pair<Key, Value>
                 */
                const typename InnerIterator::value_type & operator*();

                /**
                 * Arrow operator. Returns a pointer to the underlying value_type,
                 *  which will always be a std::pair<Key, Value>
                 */
                const typename InnerIterator::value_type* operator->();

            private:

                InnerIterator iter;
        };

        /** Easier typename to deal with if capital, also lets const_iterator share same name */
        typedef Iterator iterator;

        /** Lets const_iterator be interchangeable with "iterator" */
        typedef Iterator const_iterator;

        /**
         * @return an iterator to the beginning of this container
         */
        const_iterator begin() const;

        /**
         * @return an iterator to the end of this container
         */
        const_iterator end() const;

    private:

        std::unordered_map<Key, Value> _forward;
        std::unordered_map<Value, Key> _backward;
};

#include "invertible_map.cpp"
#endif
