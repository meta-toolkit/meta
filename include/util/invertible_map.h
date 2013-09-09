/**
 * @file invertible_map.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _INVERTIBLE_MAP_H_
#define _INVERTIBLE_MAP_H_

#include <fstream>
#include <iterator>
#include <utility>
#include <map>
#include <unordered_map>
#include <string>

namespace meta {
namespace util {

/**
 * This data structure indexes by keys as well as values, allowing constant
 *  amortized lookup time by key or value. All keys and values must be unique.
 */
template <class Key, class Value>
class invertible_map
{
    public:

        /**
         * Constructor.
         */
        invertible_map();

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
        Key get_key(const Value & value) const;

        /**
         * @return a value given a key
         */
        Value get_value(const Key & key) const;

        /**
         * @return whether the map contains the given key
         */
        bool contains_key(const Key & key) const;

        /**
         * @return whether the map contains the given value
         */
        bool contains_value(const Value & value) const;

        /**
         * Inserts a (key, value) pair into the invertible map
         */
        void insert(const Key & key, const Value & value);

        /**
         * Convenience function to add a (key, value) pair into the invertible
         * map.
         * @param pair The pair to add
         */
        void insert(const std::pair<Key, Value> & pair);

        /**
         * Saves an invertible map to disk.
         * @param filename - where to save the map
         */
        void save(const std::string & filename) const;

        /**
         * Reads a saved map from disk and loads it into the current invertible_map.
         * @param filename - the file where the map is saved
         */
        void read(const std::string & filename);

        /**
         * Frees all keys from this object.
         */
        void clear();

        /**
         * The "inner" iterator representation of the invertible_map.
         */
        typedef typename std::unordered_map<Key, Value>::const_iterator InnerIterator;

        /**
         * The invertible_map iterator is really just a wrapper for the forward (key -> value)
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

                /** the iterator of the underlying unordered_map */
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

        /** the internal map representing Key -> Value pairs */
        std::unordered_map<Key, Value> _forward;

        /** the internal map representing Value -> Key pairs */
        std::unordered_map<Value, Key> _backward;

    public:

        /**
         * Basic exception for invertible_map interactions.
         */
        class invertible_map_exception: public std::exception
        {
            public:
                
                invertible_map_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }
           
            private:
           
                std::string _error;
        };
};

}
}

#include "util/invertible_map.tcc"
#endif
