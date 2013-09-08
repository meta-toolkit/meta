/**
 * @file locking_map.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _LOCKING_MAP_H_
#define _LOCKING_MAP_H_

#include <unordered_map>
#include <memory>
#include <mutex>

#include "util/optional.h"

namespace meta {
namespace caching {

/**
 * A simple wrapper around a std::unordered_map that uses an internal
 * mutex for synchronization safety.
 */
template <class Key, class Value>
class locking_map {
    public:
        /**
         * Default constructable.
         */
        locking_map() = default;

        /**
         * locking_map may be move constructed.
         */
        locking_map(locking_map && other);

        /**
         * Locking map may be assigned.
         */
        locking_map & operator=(locking_map rhs);

        /**
         * Swaps the current instance of locking_map with the paramter.
         * @param other the map to swap with
         */
        void swap(locking_map & other);

        /**
         * Inserts a given (key, value) pair into the map.
         * @param key
         * @param value
         */
        void insert(const Key & key, const Value & value);

        /**
         * Inserts a (key, value) pair into the map, using in-place
         * construction.
         * @param args the paramters to be used for creating the (key,
         *  value) pair
         */
        template <class... Args>
        void emplace(Args &&... args);

        /**
         * Finds a value in the map. If it exists, the optional will be
         * engaged, otherwise, it will be disengaged.
         *
         * @param key the key to find the corresponding value for
         */
        util::optional<Value> find(const Key & key) const;

    private:
        std::unordered_map<Key, Value> map_;
        mutable std::mutex mutables_;
};

}
}

#include "caching/maps/locking_map.tcc"
#endif
