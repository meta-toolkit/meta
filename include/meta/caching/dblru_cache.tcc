/**
 * @file dblru_cache.tcc
 */

#include "meta/caching/dblru_cache.h"

namespace meta
{
namespace caching
{

template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>::dblru_cache(uint64_t max_size)
    : max_size_{max_size},
      current_size_{0},
      primary_{std::make_shared<Map<Key, Value>>()},
      secondary_{std::make_shared<Map<Key, Value>>()}
{
    /* nothing */
}

#if META_HAS_STD_SHARED_PTR_ATOMICS
template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>::dblru_cache(dblru_cache&& other)
    : max_size_{std::move(other.max_size_)},
      current_size_{other.current_size_.load()},
      primary_{std::atomic_load(&other.primary_)},
      secondary_{std::atomic_load(&other.secondary_)}
{
    /* nothing */
}
#else
template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>::dblru_cache(dblru_cache&& other)
    : max_size_{std::move(other.max_size_)},
      current_size_{std::move(other.current_size_)}
{
    std::lock_guard<std::mutex> lock{*other.mutables_};
    primary_ = std::move(other.primary_);
    secondary_ = std::move(other.secondary_);
}
#endif

template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>& dblru_cache<Key, Value, Map>::
operator=(dblru_cache rhs)
{
    swap(rhs);
    return *this;
}

#if META_HAS_STD_SHARED_PTR_ATOMICS
template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::swap(dblru_cache& other)
{
    std::swap(max_size_, other.max_size_);
    current_size_.store(other.current_size_.exchange(current_size_.load()));
    std::atomic_exchange(&primary_, other.primary_);
    std::atomic_exchange(&secondary_, other.secondary_);
}
#else
template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::swap(dblru_cache& other)
{
    std::lock_guard<std::mutex> other_lock{*other.mutables_};
    std::lock_guard<std::mutex> lock{*mutables_};
    std::swap(max_size_, other.max_size_);
    std::swap(current_size_, other.current_size_);
    std::swap(primary_, other.primary_);
    std::swap(secondary_, other.secondary_);
}
#endif

template <class Key, class Value, template <class, class> class Map>
std::shared_ptr<Map<Key, Value>>
dblru_cache<Key, Value, Map>::get_primary_map() const
{
#if META_HAS_STD_SHARED_PTR_ATOMICS
    return std::atomic_load(&primary_);
#else
    std::lock_guard<std::mutex> lock{*mutables_};
    return primary_;
#endif
}

template <class Key, class Value, template <class, class> class Map>
std::shared_ptr<Map<Key, Value>>
dblru_cache<Key, Value, Map>::get_secondary_map() const
{
#if META_HAS_STD_SHARED_PTR_ATOMICS
    return std::atomic_load(&secondary_);
#else
    std::lock_guard<std::mutex> lock{*mutables_};
    return secondary_;
#endif
}

template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::insert(const Key& key, const Value& value)
{
    auto map = get_primary_map();
    map->insert(key, value);
    handle_insert();
}

template <class Key, class Value, template <class, class> class Map>
template <class... Args>
void dblru_cache<Key, Value, Map>::emplace(Args&&... args)
{
    auto map = get_primary_map();
    map->emplace(std::forward<Args...>(args...));
    handle_insert();
}

template <class Key, class Value, template <class, class> class Map>
util::optional<Value> dblru_cache<Key, Value, Map>::find(const Key& key)
{
    auto primary = get_primary_map();
    auto opt = primary->find(key);
    if (opt)
        return opt;
    auto secondary = get_secondary_map();
    opt = secondary->find(key);
    if (opt)
    {
        primary->insert(key, *opt);
        handle_insert();
    }
    return opt;
}

#if META_HAS_STD_SHARED_PTR_ATOMICS
template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::handle_insert()
{
    if (current_size_.fetch_add(1) == max_size_)
    {
        auto secondary = std::atomic_load(&secondary_);
        // leaves primary_ empty, with secondary_ containing what used to
        // be in primary_
        std::atomic_exchange(&secondary_, primary_);
        std::atomic_store(&primary_, std::make_shared<Map<Key, Value>>());
        // reset counter
        current_size_.store(0);
    }
}
#else
template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::handle_insert()
{
    std::shared_ptr<Map<Key, Value>> old_secondary;
    {
        std::lock_guard<std::mutex> lock{*mutables_};
        if (++current_size_ == max_size_)
        {
            // leaves primary_ empty, with secondary_ containing what used to
            // be in primary_
            old_secondary = secondary_;
            std::swap(secondary_, primary_);
            primary_ = std::make_shared<Map<Key, Value>>();
            // reset counter
            current_size_ = 0;
        }
    }
}
#endif

template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::clear()
{
#if META_HAS_STD_SHARED_PTR_ATOMICS
    auto primary = std::atomic_load(&primary_);
    auto secondary = std::atomic_load(&secondary_);
    std::atomic_exchange(&secondary_, std::make_shared<Map<Key, Value>>());
    std::atomic_exchange(&primary_, std::make_shared<Map<Key, Value>>());
    current_size_.store(0);
#else
    std::shared_ptr<Map<Key, Value>> primary;
    std::shared_ptr<Map<Key, Value>> secondary;
    {
        std::lock_guard<std::mutex> lock{*mutables_};
        primary = primary_;
        secondary = secondary_;
        primary_ = std::make_shared<Map<Key, Value>>();
        secondary_ = std::make_shared<Map<Key, Value>>();
        current_size_ = 0;
    }
#endif
}
}
}
