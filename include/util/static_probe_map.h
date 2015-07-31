/**
 * @file static_probe_map.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_STATIC_PROBE_MAP_H_
#define META_STATIC_PROBE_MAP_H_

#include <iterator>
#include <utility>

namespace meta
{
namespace util
{

/**
 *
 */
template <class Key, class Value>
class static_probe_map
{
  public:
    /**
     * Constructor.
     * @param elems The number of elements that will be stored in this map.
     * Note that the storage required will be more than this amount in order to
     * have an acceptable load factor.
     */
    static_probe_map(uint64_t num_elems);

    Value& operator[](const Key& key);

    const Value& operator[](const Key& key) const;

    Value& at(const Key& key);

    const Value& at(const Key& key) const;

    /**
     * The "inner" iterator representation of the static_probe_map.
     */
    using InnerIterator =
        typename std::vector<std::pair<Key, Value>>::const_iterator;

    /**
     * The static_probe_map iterator is really just a wrapper for the internal
     * vector<pair<K,V>> iterator.
     */
    class Iterator
        : public std::iterator<std::bidirectional_iterator_tag, InnerIterator>
    {
      private:
        /// The iterator of the underlying vector
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
            InnerIterator save{iter};
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
            InnerIterator save{iter};
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

    Iterator find(const Key& key) const;

  private:
    /// The internal map representing Key -> Value pairs
    std::vector<std::pair<Key, Value>> table_;

    /// Hash function for this hash table
    std::hash<Key> hash_;

    /// Empty pair
    const std::pair<Key, Value> empty_;

  public:
    /**
     * @return an iterator to the beginning of this container
     */
    const_iterator begin() const
    {
        auto it = table_.begin();
        while (*it == empty_ && it != table_.end())
            ++it;
        return it;
    }

    /**
     * @return an iterator to the end of this container
     */
    const_iterator end() const
    {
        return table_.end();
    }

    /**
     * Basic exception for static_probe_map interactions.
     */
    class static_probe_map_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};
}
}

#include "util/static_probe_map.tcc"
#endif
