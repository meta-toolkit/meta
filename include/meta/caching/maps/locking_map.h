/**
 * @file locking_map.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LOCKING_MAP_H_
#define META_LOCKING_MAP_H_

#include <unordered_map>
#include <memory>
#include <mutex>

#include "meta/util/optional.h"

namespace meta
{
namespace caching
{

/**
 * A simple wrapper around a std::unordered_map that uses an internal
 * mutex for synchronization safety.
 */
template <class Key, class Value>
class locking_map
{
  public:
    /**
     * Default constructable.
     */
    locking_map() = default;

    /**
     * locking_map may be move constructed.
     */
    locking_map(locking_map&&);

    /**
     * Locking map may be assigned.
     * @return the current locking_map
     */
    locking_map& operator=(locking_map rhs);

    /**
     * Swaps the current instance of locking_map with the paramter.
     * @param other the map to swap with
     */
    void swap(locking_map& other);

    /**
     * Inserts a given (key, value) pair into the map.
     * @param key
     * @param value
     */
    void insert(const Key& key, const Value& value);

    /**
     * Inserts a (key, value) pair into the map, using in-place
     * construction.
     * @param args the parameters to be used for creating the (key,
     * value) pair
     */
    template <class... Args>
    void emplace(Args&&... args);

    /**
     * Finds a value in the map. If it exists, the optional will be
     * engaged, otherwise, it will be disengaged.
     *
     * @param key the key to find the corresponding value for
     * @return an optional that may contain the value, if found
     */
    util::optional<Value> find(const Key& key) const;

    /// iterator type for locking_maps
    using iterator = typename std::unordered_map<Key, Value>::iterator;
    /// const_iterator type for locking_maps
    using const_iterator =
        typename std::unordered_map<Key, Value>::const_iterator;

    /**
     * @return an iterator to the beginning of the map
     */
    iterator begin();

    /**
     * @return an iterator to the end of the map
     */
    iterator end();

    /**
     * @return a const_iterator to the beginning of the map
     */
    const_iterator begin() const;

    /**
     * @return a const_iterator to the end of the map
     */
    const_iterator end() const;

  private:
    /// the underlying map used for storage
    std::unordered_map<Key, Value> map_;
    /// the mutex that synchronizes accesses into the map
    mutable std::mutex mutables_;
};
}
}

#include "meta/caching/maps/locking_map.tcc"
#endif
