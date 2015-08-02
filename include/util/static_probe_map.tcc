/**
 * @file static_probe_map.tcc
 * @author Sean Massung
 */

namespace meta
{
namespace util
{
template <class Key, class Value>
static_probe_map<Key, Value>::static_probe_map(uint64_t num_elems)
    : table_(num_elems / 0.7) // load factor of 0.7
{
}

template <class Key, class Value>
Value& static_probe_map<Key, Value>::operator[](const Key& key)
{
    auto hashed = hash_(key);
    auto idx = hashed % table_.size();
    while (true)
    {
        if (table_[idx].first == uint64_t{0})
        {
            table_[idx].first = hashed;
            return table_[idx].second;
        }

        if (table_[idx].first == hashed)
            return table_[idx].second;

        idx = (idx + 1) % table_.size();
    }
}

template <class Key, class Value>
Value& static_probe_map<Key, Value>::at(const Key& key)
{
    return (*this)[key];
}

template <class Key, class Value>
const Value& static_probe_map<Key, Value>::at(const Key& key) const
{
    return (*this)[key];
}

template <class Key, class Value>
const Value& static_probe_map<Key, Value>::operator[](const Key& key) const
{
    auto hashed = hash_(key);
    auto idx = hashed % table_.size();
    while (true)
    {
        if (table_[idx].first == uint64_t{0})
            throw static_probe_map_exception{"key does not exist"};

        if (table_[idx].first == hashed)
            return table_[idx].second;

        idx = (idx + 1) % table_.size();
    }
}

template <class Key, class Value>
auto static_probe_map<Key, Value>::find(const Key& key) const -> Iterator
{
    auto hashed = hash_(key);
    auto idx = hashed % table_.size();
    while (true)
    {
        if (table_[idx].first == uint64_t{0})
            return end();

        if (table_[idx].first == hashed)
            return {table_.begin() + idx};

        idx = (idx + 1) % table_.size();
    }
}
}
}
