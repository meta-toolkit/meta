#include <atomic>
#include <type_traits>

#include "parallel/lock_free_map.h"

namespace meta {
namespace parallel {

template <class Key, class Value>
lock_free_map<Key, Value>::lock_free_map()
    : map_{std::make_shared<std::unordered_map<Key, Value>>()} { /* nothing */ }

template <class Key, class Value>
lock_free_map<Key, Value>::lock_free_map(lock_free_map && other)
    : lock_free_map{}
{
    *map_ = std::move(*other.map_);
}

template <class Key, class Value>
lock_free_map<Key, Value> &
lock_free_map<Key, Value>::operator=(lock_free_map rhs) {
    swap(rhs);
    return *this;
}

template <class Key, class Value>
void lock_free_map<Key, Value>::swap(lock_free_map & other) {
    std::swap(map_, other.map_);
}

template <class Key, class Value>
void lock_free_map<Key, Value>::insert(const Key & key, const Value & value) {
    perform_mutation([&](decltype(map_) & map) {
        (*map)[key] = value;
    });
}

template <class Key, class Value>
template <class... Args>
void lock_free_map<Key, Value>::emplace(Args &&... args) {
    perform_mutation([&](decltype(map_) & map) {
        map->emplace(std::forward<Args>(args)...);
    });
}

template <class Key, class Value>
util::optional<Value> lock_free_map<Key, Value>::find(const Key & key) const {
    return perform_operation([&](const decltype(map_) & map) {
        auto it = map->find(key);
        if(it == map->end())
            return util::optional<Value>{util::nullopt};
        return util::optional<Value>{it->second};
    });
}

template <class Key, class Value>
void lock_free_map<Key, Value>::clear() {
    perform_mutation([&](decltype(map_) & map) {
        map->clear();
    }, false);
}

template <class Key, class Value>
template <class Functor>
void lock_free_map<Key, Value>::perform_mutation(Functor && functor, bool copy)
{
    auto map = std::atomic_load(&map_);
    decltype(map) replacement;
    do {
        if(copy) {
            replacement =
                std::make_shared<std::unordered_map<Key, Value>>(*map);
        } else {
            replacement =
                std::make_shared<std::unordered_map<Key, Value>>();
        }
        functor(replacement);
    } while(!std::atomic_compare_exchange_weak(&map_, &map,
                                               std::move(replacement)));
}

template <class Key, class Value>
template <class Functor>
auto lock_free_map<Key, Value>::perform_operation(Functor && functor) const
    -> decltype(functor(nullptr))
{
    auto map = std::atomic_load(&map_);
    return functor(map);
}

}
}
