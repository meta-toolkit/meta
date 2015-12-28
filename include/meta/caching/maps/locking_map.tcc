/**
 * @file locking_map.tcc
 */

#include "meta/caching/maps/locking_map.h"

namespace meta
{
namespace caching
{

template <class Key, class Value>
locking_map<Key, Value>::locking_map(locking_map&& other)
    : map_{std::move(other.map_)}
{/* nothing */
}

template <class Key, class Value>
locking_map<Key, Value>& locking_map<Key, Value>::operator=(locking_map rhs)
{
    swap(rhs);
    return *this;
}

template <class Key, class Value>
void locking_map<Key, Value>::swap(locking_map& other)
{
    std::swap(map_, other.map_);
}

template <class Key, class Value>
void locking_map<Key, Value>::insert(const Key& key, const Value& value)
{
    std::lock_guard<std::mutex> lock{mutables_};
    map_[key] = value;
}

template <class Key, class Value>
template <class... Args>
void locking_map<Key, Value>::emplace(Args&&... args)
{
    std::lock_guard<std::mutex> lock{mutables_};
    map_.emplace(std::forward<Args>(args)...);
}

template <class Key, class Value>
util::optional<Value> locking_map<Key, Value>::find(const Key& key) const
{
    std::lock_guard<std::mutex> lock{mutables_};
    auto it = map_.find(key);
    if (it == map_.end())
        return {util::nullopt};
    return {it->second};
}

template <class Key, class Value>
auto locking_map<Key, Value>::begin() -> iterator
{
    return map_.begin();
}

template <class Key, class Value>
auto locking_map<Key, Value>::end() -> iterator
{
    return map_.end();
}

template <class Key, class Value>
auto locking_map<Key, Value>::begin() const -> const_iterator
{
    return map_.begin();
}

template <class Key, class Value>
auto locking_map<Key, Value>::end() const -> const_iterator
{
    return map_.end();
}
}
}
