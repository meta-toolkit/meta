/**
 * @file dblru_cache.tcc
 */

#include "caching/dblru_cache.h"

namespace meta {
namespace caching {

template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>::dblru_cache(uint64_t max_size)
    : max_size_{max_size},
      current_size_{0},
      primary_{std::make_shared<Map<Key, Value>>()},
      secondary_{std::make_shared<Map<Key, Value>>()} { /* nothing */ }

template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>::dblru_cache(dblru_cache && other)
    : max_size_{std::move(other.max_size_)},
      current_size_{other.current_size_.load()},
      primary_{std::atomic_load(&other.primary_)},
      secondary_{std::atomic_load(&other.secondary_)} { /* nothing */ }

template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map> &
dblru_cache<Key, Value, Map>::operator=(dblru_cache rhs) {
    swap(rhs);
    return *this;
}

template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::swap(dblru_cache & other) {
    std::swap(max_size_, other.max_size_);
    current_size_.store(other.current_size_.exchange(current_size_.load()));
    std::atomic_exchange(&primary_, other.primary_);
    std::atomic_exchange(&secondary_, other.secondary_);
}

template <class Key, class Value, template <class, class> class Map>
void
dblru_cache<Key, Value, Map>::insert(const Key & key, const Value & value) {
    auto map = std::atomic_load(&primary_);
    map->insert(key, value);
    handle_insert();
}

template <class Key, class Value, template <class, class> class Map>
template <class... Args>
void dblru_cache<Key, Value, Map>::emplace(Args &&... args) {
    auto map = std::atomic_load(&primary_);
    map->emplace(std::forward<Args...>(args...));
    handle_insert();
}

template <class Key, class Value, template <class, class> class Map>
util::optional<Value> dblru_cache<Key, Value, Map>::find(const Key & key) {
    auto primary = std::atomic_load(&primary_);
    auto opt = primary->find(key);
    if(opt)
        return opt;
    auto secondary = std::atomic_load(&secondary_);
    opt = secondary->find(key);
    if(opt) {
        primary->insert(key, *opt);
        handle_insert();
    }
    return opt;
}

template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::handle_insert() {
    if(current_size_.fetch_add(1) == max_size_) {
        auto secondary = std::atomic_load(&secondary_);
        // leaves primary_ empty, with secondary_ containing what used to
        // be in primary_
        std::atomic_exchange(&secondary_, primary_);
        std::atomic_store(&primary_, std::make_shared<Map<Key, Value>>());
        // reset counter
        current_size_.store(0);

        for (const auto & p : *secondary) {
            for (auto & callback : drop_callbacks_) {
                callback(p.first, p.second);
            }
        }
    }
}

template <class Key, class Value, template <class, class> class Map>
template <class Functor>
void dblru_cache<Key, Value, Map>::on_drop(Functor && functor) {
    drop_callbacks_.emplace_back(std::forward<Functor>(functor));
}

template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::clear() {
    auto primary = std::atomic_load(&primary_);
    auto secondary = std::atomic_load(&secondary_);
    std::atomic_exchange(&secondary_, std::make_shared<Map<Key, Value>>());
    std::atomic_exchange(&primary_, std::make_shared<Map<Key, Value>>());
    current_size_.store(0);

    for (auto & callback : drop_callbacks_) {
        for (const auto & p : *primary)
            callback(p.first, p.second);
        for (const auto & p : *secondary)
            callback(p.first, p.second);
    }
}

}
}
