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

#include "caching/unordered_map_cache.h"
#include "parallel/lock_free_map.h"
#include "util/optional.h"

namespace meta {
namespace caching {

/**
 *
 * @see https://issues.apache.org/jira/browse/LUCENE-2075
 */
template <class Key, class Value,
          template <class, class> class Map = parallel::lock_free_map>
class dblru_cache {
    public:
        dblru_cache(uint64_t max_size);

        dblru_cache(dblru_cache &&);
        dblru_cache & operator=(dblru_cache rhs);

        ~dblru_cache() = default;

        void swap(dblru_cache & other);

        void insert(const Key & key, const Value & value);

        template <class... Args>
        void emplace(Args &&... args);

        util::optional<Value> find(const Key & key);
    private:
        void handle_insert();

        uint64_t max_size_;
        std::atomic<uint64_t> current_size_;
        Map<Key, Value> primary_;
        Map<Key, Value> secondary_;
};

template <class Key, class Value>
using default_dblru_cache = dblru_cache<Key, Value>;

template <class Key, class Value>
using lock_free_dblru_cache = dblru_cache<Key, Value>;

template <class Key, class Value>
using unordered_dblru_cache = dblru_cache<Key, Value,
                                          caching::unordered_map_cache>;

}
}

#include "caching/dblru_cache.tcc"
#endif
