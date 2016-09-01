/**
 * @file dblru_cache.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DBLRU_CACHE_H_
#define META_DBLRU_CACHE_H_

#include "meta/config.h"

#if META_HAS_STD_SHARED_PTR_ATOMICS
#include <atomic>
#else
#include "meta/util/shim.h"
#include <mutex>
#endif
#include <functional>
#include <vector>

#include "meta/caching/maps/locking_map.h"
#include "meta/util/optional.h"

namespace meta
{
namespace caching
{

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
class dblru_cache
{
  public:
    /**
     * Constructs a dlbru_cache with a given fixed size.
     * @param max_size the maximum allowed size for this cache
     */
    dblru_cache(uint64_t max_size);

    /**
     * dblru_cache may be move constructed.
     */
    dblru_cache(dblru_cache&&);

    /**
     * dlbru_cache may be be assigned.
     * @return the current dblru_cache
     */
    dblru_cache& operator=(dblru_cache);

    /**
     * Default destructor.
     */
    ~dblru_cache() = default;

    /**
     * Swaps the current dlbru_cache with a given one.
     * @param other the cache to swap with
     */
    void swap(dblru_cache& other);

    /**
     * Insert a given (key, value) pair into the cache.
     * @param key
     * @param value
     */
    void insert(const Key& key, const Value& value);

    /**
     * Inserts a key value pair into the cache using in-place
     * construction if possible.
     *
     * @param args the list of arguments to forward to constructing
     * the (key, value) pair
     */
    template <class... Args>
    void emplace(Args&&... args);

    /**
     * Finds a value in the cache. If it exists, the optional will be
     * engaged, otherwise, it will be disengaged.
     *
     * @param key the key to find the corresponding value for
     * @return an optional that may contain the value, if found
     */
    util::optional<Value> find(const Key& key);

    /** Empties the cache. */
    void clear();

  private:
    /**
     * Helper function to ensure that the primary and secondary map
     * swapping occurs at the correct moment.
     */
    void handle_insert();

    /**
     * Gets the primary map.
     */
    std::shared_ptr<Map<Key, Value>> get_primary_map() const;

    /**
     * Gets the secondary map.
     */
    std::shared_ptr<Map<Key, Value>> get_secondary_map() const;

    /**
     * The maximum allowed size for the cache.
     */
    uint64_t max_size_;

/**
 * The current size of the primary map.
 */
#if META_HAS_STD_SHARED_PTR_ATOMICS
    /// the current size of the map
    std::atomic<uint64_t> current_size_;
#else
    uint64_t current_size_;
    std::unique_ptr<std::mutex> mutables_{make_unique<std::mutex>()};
#endif

    /**
     * The primary map.
     */
    std::shared_ptr<Map<Key, Value>> primary_;

    /**
     * The secondary map.
     */
    std::shared_ptr<Map<Key, Value>> secondary_;
};

/**
 * The default instantiation of a dblru_cache.
 */
template <class Key, class Value>
using default_dblru_cache = dblru_cache<Key, Value>;
}
}

#include "meta/caching/dblru_cache.tcc"
#endif
