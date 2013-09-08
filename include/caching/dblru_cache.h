/**
 * @file dblru_cache.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _DBLRU_CACHE_H_
#define _DBLRU_CACHE_H_

#include "caching/maps/locking_map.h"
#include "caching/maps/lock_free_map.h"
#include "util/optional.h"

namespace meta {
namespace caching {

/**
 * A double-barrel approach at a LRU cache.
 *
 * Uses two Maps, primary and secondary, for implementation. A find will
 * search in the primary and, if found, return the found value. Otherwise,
 * it will search the secondary---if found, that value is promoted to the
 * primary map and the value returned. Otherwise, the value does not exist.
 *
 * After a fixed number of inserts into the primary, the secondary is
 * emptied and swapped with the primary. This ensures that things that have
 * been less recently used are dropped.
 *
 * It is assumed that the Maps are internally synchronized (i.e., they
 * contain a mutex or have some other way of guaranteeing concurrency
 * safety).
 *
 * @see https://issues.apache.org/jira/browse/LUCENE-2075
 */
template <class Key, class Value,
          template <class, class> class Map = locking_map>
class dblru_cache {
    public:
        /**
         * Constructs a dlbru_cache with a given fixed size.
         * @param max_size the maximum allowed size for this cache
         */
        dblru_cache(uint64_t max_size);

        /**
         * dblru_cache may be move constructed.
         */
        dblru_cache(dblru_cache &&);

        /**
         * dlbru_cache may be be assigned.
         */
        dblru_cache & operator=(dblru_cache rhs);

        /**
         * Default destructor.
         */
        ~dblru_cache() = default;

        /**
         * Swaps the current dlbru_cache with a given one.
         * @param other the cache to swap with
         */
        void swap(dblru_cache & other);

        /**
         * Insert a given (key, value) pair into the cache.
         * @param key
         * @param value
         */
        void insert(const Key & key, const Value & value);

        /**
         * Inserts a key value pair into the cache using in-place
         * construction if possible.
         *
         * @param args the list of arguments to forward to constructing
         *  the (key, value) pair
         */
        template <class... Args>
        void emplace(Args &&... args);

        /**
         * Finds a value in the cache. If it exists, the optional will be
         * engaged, otherwise, it will be disengaged.
         *
         * @param key the key to find the corresponding value for
         */
        util::optional<Value> find(const Key & key);
    private:
        /**
         * Helper function to ensure that the primary and secondary map
         * swapping occurs at the correct moment.
         */
        void handle_insert();

        /**
         * The maximum allowed size for the cache.
         */
        uint64_t max_size_;

        /**
         * The current size of the primary map.
         */
        std::atomic<uint64_t> current_size_;

        /**
         * The primary map.
         */
        Map<Key, Value> primary_;

        /**
         * The secondary map.
         */
        Map<Key, Value> secondary_;
};

/**
 * The default instantiation of a dblru_cache.
 */
template <class Key, class Value>
using default_dblru_cache = dblru_cache<Key, Value>;

/**
 * A lock-free version of the dblru_cache.
 */
template <class Key, class Value>
using lock_free_dblru_cache = dblru_cache<Key, Value, lock_free_map>;

/**
 * A locking version of the dblru_cache.
 */
template <class Key, class Value>
using locking_dblru_cache = dblru_cache<Key, Value, locking_map>;

}
}

#include "caching/dblru_cache.tcc"
#endif
