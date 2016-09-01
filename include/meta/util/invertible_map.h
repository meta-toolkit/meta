/**
 * @file invertible_map.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INVERTIBLE_MAP_H_
#define META_INVERTIBLE_MAP_H_

#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>

#include "meta/config.h"

namespace meta
{
namespace util
{

/**
 * This data structure indexes by keys as well as values, allowing constant
 * amortized lookup time by key or value. All keys and values must be unique.
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
     * @param value The value to search by
     * @return a key given a value
     */
    Key get_key(const Value& value) const;

    /**
     * @param key The key to search by
     * @return a value given a key
     */
    Value get_value(const Key& key) const;

    /**
     * @param key The key to search for
     * @return whether the map contains the given key
     */
    bool contains_key(const Key& key) const;

    /**
     * @param value The value to search for
     * @return whether the map contains the given value
     */
    bool contains_value(const Value& value) const;

    /**
     * Inserts a (key, value) pair into the invertible map
     * @param key The key to insert
     * @param value The value to insert
     */
    void insert(const Key& key, const Value& value);

    /**
     * Convenience function to add a (key, value) pair into the invertible
     * map.
     * @param pair The pair to add
     */
    void insert(const std::pair<Key, Value>& pair);

    /**
     * Frees all keys from this object.
     */
    void clear();

    /**
     * The "inner" iterator representation of the invertible_map.
     */
    typedef
        typename std::unordered_map<Key, Value>::const_iterator InnerIterator;

    /**
     * The invertible_map iterator is really just a wrapper for the forward
     * (key -> value) unordered_map iterator. Use this iterator class the
     * same way you'd use it on an unordered_map.
     */
    class Iterator
        : public std::iterator<std::bidirectional_iterator_tag, InnerIterator>
    {
      private:
        /// The iterator of the underlying unordered_map
        InnerIterator iter;

      public:
        /// Constructor.
        Iterator()
        {
            /* nothing */
        }

        /// Copy constructor.
        Iterator(const InnerIterator& other) : iter{other}
        {
            /* nothing */
        }

        /// Pre-Increment.
        Iterator& operator++()
        {
            ++iter;
            return *this;
        }

        /// Post-increment.
        Iterator operator++(int)
        {
            InnerIterator save = iter;
            ++iter;
            return Iterator{save};
        }

        /// Pre-decrement.
        Iterator& operator--()
        {
            --iter;
            return *this;
        }

        /// Post-decrement.
        Iterator operator--(int)
        {
            InnerIterator save = iter;
            --iter;
            return Iterator{save};
        }

        /// Iterator equality.
        bool operator==(const Iterator& other)
        {
            return iter == other.iter;
        }

        /// Iterator inequality.
        bool operator!=(const Iterator& other)
        {
            return iter != other.iter;
        }

        /**
         * Dereference operator. Returns the underlying value_type,
         *  which will always be a std::pair<Key, Value>
         * @return a reference to the value of the object that is dereferenced
         */
        const typename InnerIterator::value_type& operator*()
        {
            return *iter;
        }

        /**
         * Arrow operator. Returns a pointer to the underlying
         * value_type, which will always be a std::pair<Key, Value>
         * @return a pointer to the value of the object that is dereferenced
         */
        const typename InnerIterator::value_type* operator->()
        {
            return &(*iter);
        }
    };

    /**
     * Easier typename to deal with if capital, also lets const_iterator
     * share same name
     */
    typedef Iterator iterator;

    /// Lets const_iterator be interchangeable with "iterator"
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
    /// The internal map representing Key -> Value pairs
    std::unordered_map<Key, Value> forward_;

    /// The internal map representing Value -> Key pairs
    std::unordered_map<Value, Key> backward_;
};

/**
 * Basic exception for invertible_map interactions.
 */
class invertible_map_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "meta/util/invertible_map.tcc"
#endif
