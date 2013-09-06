#include <atomic>

#include "parallel/lock_free_map.h"

namespace meta {
namespace parallel {

template <class Key, class Value>
lock_free_map<Key, Value>::lock_free_map() {
    map_ = std::make_shared<std::unordered_map<Key, Value>>();
}

template <class Key, class Value>
void lock_free_map<Key, Value>::insert(const Key & key, const Value & value) {
    perform_mutation([&](decltype(map_) & map) {
        map->insert(key, value);
    });
}

template <class Key, class Value>
template <class... Args>
void lock_free_map<Key, Value>::emplace(Args &&... args) {
    perform_mutation([&](decltype(map_) & map) {
        map->emplace(std::forward<Args...>(args...));
    });
}

template <class Key, class Value>
bool lock_free_map<Key, Value>::exists(const Key & key) const {
    return perform_operation([&](decltype(map_) & map) {
        return map->find(key) != map->end();
    });
}

template <class Key, class Value>
Value lock_free_map<Key, Value>::find(const Key & key) const {
    return perform_operation([&](decltype(map_) & map) {
        return map->at(key);
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
    perform_operation([&](decltype(map_) & map) {
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
    });
}

template <class Key, class Value>
template <class Functor>
auto lock_free_map<Key, Value>::perform_operation(Functor && functor)
    -> typename std::result_of<Functor>::type
{
    auto map = std::atomic_load(&map_);
    return functor(map);
}

}
}
