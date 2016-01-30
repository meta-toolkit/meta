#include "meta/caching/no_evict_cache.h"

namespace meta
{
namespace caching
{

template <class Key, class Value>
void no_evict_cache<Key, Value>::insert(const Key& key, const Value& value)
{
    std::lock_guard<std::mutex> lock{*mutables_};
    if (key >= values_.size())
        values_.resize(key + 1);
    values_[key] = util::optional<Value>{value};
}

template <class Key, class Value>
util::optional<Value> no_evict_cache<Key, Value>::find(const Key& key) const
{
    if (key >= values_.size())
        return util::nullopt;
    return values_[key];
}

template <class Key, class Value>
void no_evict_cache<Key, Value>::clear()
{
    values_.clear();
}
}
}
