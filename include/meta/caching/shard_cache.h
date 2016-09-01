/**
 * @file shard_cache.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SHARD_CACHE_H_
#define META_SHARD_CACHE_H_

#include <mutex>
#include <vector>

#include "meta/caching/dblru_cache.h"
#include "meta/caching/splay_cache.h"
#include "meta/config.h"
#include "meta/util/optional.h"

namespace meta
{
namespace caching
{

/**
 * A simple sharding-based approach for increasing concurrency within a
 * cache. Wraps a given number of Maps, each used to contain a segment of
 * the keyspace. It is assumed that the Map class is self-synchronizing
 * (i.e., it has a mutex or other concurrency safety mechanism built in).
 */
template <class Key, class Value, template <class, class> class Map>
class generic_shard_cache
{
  public:
    /**
     * Constructs a shard cache with the given number of shards, passing
     * any additional parameters to the underlying Map classes created.
     *
     * @param shards the number of shards to create
     * @param args the remaining arguments to be used in creating the Map
     * for each shard
     */
    template <class... Args>
    generic_shard_cache(uint8_t shards, Args&&... args);

    /**
     * generic_shard_cache may be move constructed
     */
    generic_shard_cache(generic_shard_cache&&) = default;

    /**
     * generic_shard_cache may be move assigned
     */
    generic_shard_cache& operator=(generic_shard_cache&&) = default;

    /**
     * Default destructor.
     */
    ~generic_shard_cache() = default;

    /**
     * Inserts a given (key, value) pair into the cache.
     * @param key
     * @param value
     */
    void insert(const Key& key, const Value& value);

    /**
     * Finds a value in the cache. If it exists, the optional will be
     * engaged, otherwise, it will be disengaged.
     *
     * @param key the key to find the corresponding value for
     * @return an optional that may contain the value, if found
     */
    util::optional<Value> find(const Key& key);

  private:
    /**
     * The Map for each shard.
     */
    std::vector<Map<Key, Value>> shards_;

    /**
     * The hash function used for determining which shard a key
     * belongs to.
     */
    std::hash<Key> hasher_;
};

/**
 * A sharding cache that uses splay_caches as the internal map.
 */
template <class Key, class Value>
using splay_shard_cache = generic_shard_cache<Key, Value, splay_cache>;

/**
 * A sharding cache that uses a default_dblru_cache as the internal map.
 */
template <class Key, class Value>
using dblru_shard_cache = generic_shard_cache<Key, Value, default_dblru_cache>;
}
}

#include "meta/caching/shard_cache.tcc"
#endif
