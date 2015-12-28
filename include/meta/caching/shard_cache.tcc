/**
 * @file shard_cache.tcc
 */

#include "meta/caching/shard_cache.h"

namespace meta
{
namespace caching
{

template <class Key, class Value, template <class, class> class Map>
template <class... Args>
generic_shard_cache<Key, Value, Map>::generic_shard_cache(uint8_t shards,
                                                          Args&&... args)
{
    for (uint8_t i = 0; i < shards; ++i)
        shards_.emplace_back(std::forward<Args>(args)...);
}

template <class Key, class Value, template <class, class> class Map>
void generic_shard_cache<Key, Value, Map>::insert(const Key& key,
                                                  const Value& value)
{
    auto shard = hasher_(key) % shards_.size();
    shards_[shard].insert(key, value);
}

template <class Key, class Value, template <class, class> class Map>
util::optional<Value> generic_shard_cache<Key, Value, Map>::find(const Key& key)
{
    auto shard = hasher_(key) % shards_.size();
    return shards_[shard].find(key);
}
}
}
