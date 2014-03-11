/**
 * @file no_evict_cache.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NO_EVICT_CACHE_H_
#define META_NO_EVICT_CACHE_H_

#include <deque>
#include <memory>
#include <mutex>

#include "meta.h"
#include "util/optional.h"
#include "util/shim.h"

namespace meta {
namespace caching {

/**
 * An incredibly simple "cache" that simply keeps everything in memory.
 * Useful for when the dataset is small enough that we have enough RAM to
 * fit it in-core.
 */
template <class Key, class Value>
class no_evict_cache {
    public:
        static_assert(
            (std::is_integral<Key>::value
             || std::is_base_of<util::numeric, Key>::value),
            "Key for no_evict_cache must be a numeric type"
        );

        /**
         * Inserts the given key, value pair into the cache. May be slow
         * while the cache is initially populated.
         * @param key the key to insert
         * @param value the value to insert
         */
        void insert(const Key & key, const Value & value);

        /**
         * Finds the value associated with the given key.
         * @param key the key to find the associated value for, if it exists
         */
        util::optional<Value> find(const Key & key) const;

    private:

        /**
         * Mutex for locking operations.
         */
        std::unique_ptr<std::mutex> mutables_{make_unique<std::mutex>()};

        /**
         * Contains all of the values inserted thus far. Never shrinks.
         */
        std::deque<util::optional<Value>> values_;
};

}
}
#include "caching/no_evict_cache.tcc"
#endif
