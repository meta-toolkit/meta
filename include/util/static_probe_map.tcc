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
    auto idx = hash_(key) % table_.size();
    while (true)
    {
        if (table_[idx] == empty_)
        {
            table_[idx].first = key;
            return table_[idx].second;
        }

        if (table_[idx].first == key)
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
    auto idx = hash_(key) % table_.size();
    while (true)
    {
        if (table_[idx] == empty_)
            throw static_probe_map_exception{"key does not exist"};

        if (table_[idx].first == key)
            return table_[idx].second;

        idx = (idx + 1) % table_.size();
    }
}

template <class Key, class Value>
auto static_probe_map<Key, Value>::find(const Key& key) const -> Iterator
{
    auto idx = hash_(key) % table_.size();
    while (true)
    {
        if (table_[idx] == empty_)
            return end();

        if (table_[idx].first == key)
            return {table_.begin() + idx};

        idx = (idx + 1) % table_.size();
    }
}
}
}
