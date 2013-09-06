/**
 * @file shard_cache.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _SHARD_CACHE_H_
#define _SHARD_CACHE_H_

#include <mutex>
#include <vector>

#include "caching/dblru_cache.h"
#include "caching/splay_cache.h"
#include "caching/unordered_map_cache.h"
#include "parallel/lock_free_map.h"

namespace meta {
namespace caching {

template <class Key, class Value, template <class, class> class Map>
class generic_shard_cache {
    public:
        template <class... Args>
        generic_shard_cache(uint8_t shards, Args &&... args);

        generic_shard_cache(generic_shard_cache &&) = default;
        generic_shard_cache & operator=(generic_shard_cache &&) = default;
        ~generic_shard_cache() = default;

        void insert(const Key & key, const Value & value);
        bool exists(const Key & key);
        Value find(const Key & key);

    private:
        std::vector<Map<Key, Value>> shards_;
        std::hash<Key> hasher_;
};

template <class Key, class Value>
using splay_shard_cache =
    generic_shard_cache<Key, Value, caching::splay_cache>;

template <class Key, class Value>
using lock_free_dblru_shard_cache =
    generic_shard_cache<Key, Value, caching::lock_free_dblru_cache>;

template <class Key, class Value>
using unordered_dblru_shard_cache =
    generic_shard_cache<Key, Value, caching::unordered_dblru_cache>;

}
}

#include "caching/shard_cache.tcc"
#endif
