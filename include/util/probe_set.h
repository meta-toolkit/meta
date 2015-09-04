/**
 * @file probe_set.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_PROBE_SET_H_
#define META_UTIL_PROBE_SET_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace meta
{
namespace util
{

namespace detail
{
/**
 * @param num The number to round to the next prime number
 * @return a prime number greater than num
 */
inline uint64_t next_prime(uint64_t num)
{
    // list of primes for resizing. "borrowed" from boost::unordered.
    static uint64_t primes[]
        = {17ul,         29ul,         37ul,        53ul,        67ul,
           79ul,         97ul,         131ul,       193ul,       257ul,
           389ul,        521ul,        769ul,       1031ul,      1543ul,
           2053ul,       3079ul,       6151ul,      12289ul,     24593ul,
           49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
           1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
           50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
           1610612741ul, 3221225473ul, 4294967291ul};

    auto prime = std::upper_bound(std::begin(primes), std::end(primes), num);
    if (prime == std::end(primes))
        --prime;
    return *prime;
}
}

/**
 * An **insert-only** linear probing hash set. The keys are stored in
 * contiguous memory, and the table itself maps to an index into the key
 * storage.
 *
 * The primary use case for this is for storing in-memory chunks of
 * postings data during indexing, but it could easily be used in other
 * places.
 */
template <class Key, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class probe_set
{
  public:
    /**
     * An object to represent load factors for the table. It is represented
     * as a fraction instead of a floating point number to avoid floating
     * point operations when checking for the resize condition.
     *
     * Its default value is 3/4, meaning that the table will resize when
     * more than 3/4 of the cells would become occupied after an insert.
     */
    struct load_factor
    {
        uint64_t numerator = 3;
        uint64_t denominator = 4;
    };

    /**
     * Constructs an empty probe_set.
     * @param alpha The desired load factor (optional)
     */
    probe_set(load_factor alpha = {})
        : table_(17), occupancy_(17), alpha_(alpha)
    {
        // nothing
    }

    /**
     * An iterator over the set. Elements cannot be modified through this
     * iterator.
     */
    class const_iterator
    {
      public:
        friend probe_set;

        /**
         * Iterators can be default constructed.
         */
        const_iterator() = default;

        /**
         * @return the current iterator
         */
        const_iterator& operator++()
        {
            do
            {
                ++idx_;
            } while (idx_ < parent_->occupancy_.size()
                     && !parent_->occupancy_[idx_]);
            return *this;
        }

        /**
         * @return the key pointed to by this iterator
         */
        const Key& operator*() const
        {
            return parent_->keys_[parent_->table_[idx_]];
        }

        /**
         * @return a pointer to the key pointed to by this iterator
         */
        const Key* operator->() const
        {
            return &parent_->keys_[parent_->table_[idx_]];
        }

        /**
         * @param rhs The other iterator
         * @return whether this iterator is equal to the other
         */
        bool operator==(const const_iterator& rhs)
        {
            return parent_ == rhs.parent_ && idx_ == rhs.idx_;
        }

        /**
         * @param rhs The other iterator
         * @return whether this iterator is not equal to the other
         */
        bool operator!=(const const_iterator& rhs)
        {
            return !(*this == rhs);
        }

        /**
         * @return the index of this key in the keys array
         */
        std::size_t index() const
        {
            return parent_->table_[idx_];
        }

      private:
        /**
         * The private constructor used by the probe_set to create its
         * begin and end iterators.
         *
         * @param parent The probe set to iterate over
         * @param idx The starting index into the table
         */
        const_iterator(const probe_set* parent, std::size_t idx)
            : parent_{parent}, idx_{idx}
        {
            if (idx < parent_->occupancy_.size() && !parent_->occupancy_[idx])
                ++(*this);
        }

        /// The probe_set this iterator is iterating over
        const probe_set* parent_;

        /// The current index into the table
        std::size_t idx_;
    };
    /// Regular iterators are the same as const_iterators.
    using iterator = const_iterator;

    /**
     * @return an iterator to the beginning of the set
     */
    const_iterator begin() const
    {
        return {this, 0};
    }

    /**
     * @return an iterator to the end of the set
     */
    const_iterator end() const
    {
        return {this, occupancy_.size()};
    }

    /**
     * @param key an rvalue reference to the key to be inserted into the
     * table
     * @return an iterator to the item inserted
     */
    iterator emplace(Key&& key)
    {
        if (alpha_.denominator * (keys_.size() + 1) >= alpha_.numerator
                                                           * occupancy_.size())
            resize();

        auto idx = hash_(key) % occupancy_.size();
        while (occupancy_[idx])
            idx = (idx + 1) % occupancy_.size();

        occupancy_[idx] = true;
        table_[idx] = keys_.size();

        // hack for a 1.5x resizing vector on all implementations
        if (keys_.size() == keys_.capacity())
            keys_.reserve(keys_.size() + (keys_.size() + 1) / 2);
        keys_.emplace_back(std::move(key));

        return {this, idx};
    }

    /**
     * @param key a reference to the key to be inserted into the table
     * @return an iterator to the item inserted
     */
    iterator insert(const Key& key)
    {
        Key to_insert{key};
        return emplace(std::move(to_insert));
    }

    /**
     * @param key The key to locate in the set
     * @return an iterator to the key, if it exists, or to the end if it
     * doesn't
     */
    const_iterator find(const Key& key) const
    {
        auto idx = hash_(key) % occupancy_.size();
        while (occupancy_[idx] && !(equal_(keys_[table_[idx]], key)))
            idx = (idx + 1) % occupancy_.size();

        if (!occupancy_[idx])
            return end();

        return {this, idx};
    }

    /**
     * Empties the set. This releases the memory associated with the keys
     * in the set, but keeps the memory associated with the table itself.
     */
    void clear()
    {
        // actually free all of the data, but keep around the actual table
        // itself
        std::vector<Key>{}.swap(keys_);

        // remember to mark everything as unoccupied
        std::fill(occupancy_.begin(), occupancy_.end(), false);
    }

    /**
     * @param other The set to swap with
     */
    void swap(probe_set& other)
    {
        using std::swap;
        swap(table_, other.table_);
        swap(occupancy_, other.occupancy_);
        swap(keys_, other.keys_);
        swap(hash_, other.hash_);
        swap(equal_, other.equal_);
    }

    /**
     * @return whether the table is empty
     */
    bool empty() const
    {
        return keys_.empty();
    }

    /**
     * @return the current number of keys in the set
     */
    std::size_t size() const
    {
        return keys_.size();
    }

    /**
     * @return the current number of elements that can be stored in the
     * table itself
     */
    std::size_t capacity() const
    {
        return occupancy_.size();
    }

    /**
     * @return an estimate for the number of heap allocated bytes used by
     * the container
     */
    std::size_t bytes_used() const
    {
        return sizeof(std::size_t) * table_.capacity() + occupancy_.capacity()
               + sizeof(Key) * keys_.capacity() + sizeof(load_factor)
               + sizeof(Hash) + sizeof(KeyEqual);
    }

    /**
     * @return the maximum allowed load factor for this table
     */
    const load_factor& max_load_factor() const
    {
        return alpha_;
    }

    /**
     * This empties the hash table and returns the contiguous storage used
     * to store the keys.
     * @return all of the keys in the table
     */
    std::vector<Key> extract_keys()
    {
        auto res = std::move(keys_);
        clear();
        return res;
    }

  private:
    /**
     * Increases the capacity of the table by resizing to twice the size
     * rounded up to the closest prime number.
     */
    void resize()
    {
        std::vector<bool> newocc(detail::next_prime(occupancy_.size() * 2));
        std::vector<std::size_t> newtable(newocc.size());

        for (std::size_t idx = 0; idx < occupancy_.size(); ++idx)
        {
            if (occupancy_[idx])
            {
                auto& key = keys_[table_[idx]];
                auto nidx = hash_(key) % newocc.size();

                while (newocc[nidx])
                    nidx = (nidx + 1) % newocc.size();

                newocc[nidx] = true;
                newtable[nidx] = table_[idx];
            }
        }

        using std::swap;
        swap(newocc, occupancy_);
        swap(newtable, table_);
    }

    /// The table itself, which maps to indices into the keys storage
    std::vector<std::size_t> table_;
    /// Whether a specific location in the table is occupied or not
    std::vector<bool> occupancy_;
    /// The contiguous storage used for holding the keys
    std::vector<Key> keys_;
    /// The maximum allowed load factor for the table
    load_factor alpha_;
    /// The hash function used for hashing the keys
    Hash hash_;
    /// The comparator used for testing keys for equality
    KeyEqual equal_;
};
}
}
#endif
