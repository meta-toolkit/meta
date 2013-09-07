#include "caching/unordered_map_cache.h"

namespace meta {
namespace caching {

template <class Key, class Value>
unordered_map_cache<Key, Value>::unordered_map_cache(
        unordered_map_cache && other)
    : map_{std::move(other.map_)} { /* nothing */ }

template <class Key, class Value>
unordered_map_cache<Key, Value> &
unordered_map_cache<Key, Value>::operator=(unordered_map_cache rhs) {
    swap(rhs);
    return *this;
}

template <class Key, class Value>
void unordered_map_cache<Key, Value>::swap(unordered_map_cache & other) {
    std::swap(map_, other.map_);
}

template <class Key, class Value>
void
unordered_map_cache<Key, Value>::insert(const Key & key, const Value & value) {
    std::lock_guard<std::mutex> lock{mutables_};
    map_[key] = value;
}

template <class Key, class Value>
template <class... Args>
void unordered_map_cache<Key, Value>::emplace(Args &&... args) {
    std::lock_guard<std::mutex> lock{mutables_};
    map_.emplace(std::forward<Args>(args)...);
}

template <class Key, class Value>
util::optional<Value> unordered_map_cache<Key, Value>::find(const Key & key) const {
    std::lock_guard<std::mutex> lock{mutables_};
    auto it = map_.find(key);
    if(it == map_.end())
        return {util::nullopt};
    return {it->second};
}

}
}
