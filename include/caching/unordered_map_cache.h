/**
 * @file splay_cache.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _UNORDERED_MAP_CACHE_H_
#define _UNORDERED_MAP_CACHE_H_

#include <unordered_map>
#include <memory>
#include <mutex>

namespace meta {
namespace caching {

template <class Key, class Value>
class unordered_map_cache {
    public:
        unordered_map_cache() = default;

        unordered_map_cache(unordered_map_cache && other);

        unordered_map_cache & operator=(unordered_map_cache rhs);

        void swap(unordered_map_cache & other);

        void insert(const Key & key, const Value & value);

        template <class... Args>
        void emplace(Args &&... args);

        bool exists(const Key & key) const;

        const Value & find(const Key & key) const;

    private:
        std::unordered_map<Key, Value> map_;
        mutable std::mutex mutables_;
};

}
}

#include "caching/unordered_map_cache.tcc"
#endif
