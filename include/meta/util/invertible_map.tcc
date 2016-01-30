/**
 * @file invertible_map.tcc
 * @author Sean Massung
 */

#include <sstream>

namespace meta
{
namespace util
{

template <class Key, class Value>
invertible_map<Key, Value>::invertible_map()
{
    /* nothing */
}

template <class Key, class Value>
bool invertible_map<Key, Value>::empty() const
{
    return forward_.empty();
}

template <class Key, class Value>
void invertible_map<Key, Value>::clear()
{
    forward_.clear();
    backward_.clear();
}

template <class Key, class Value>
size_t invertible_map<Key, Value>::size() const
{
    return forward_.size();
}

template <class Key, class Value>
Key invertible_map<Key, Value>::get_key(const Value& value) const
{
    auto it = backward_.find(value);
    if (it == backward_.end())
        return Key();
    return it->second;
}

template <class Key, class Value>
Value invertible_map<Key, Value>::get_value(const Key& key) const
{
    auto it = forward_.find(key);
    if (it == forward_.end())
        return Value{};
    return it->second;
}

template <class Key, class Value>
bool invertible_map<Key, Value>::contains_key(const Key& key) const
{
    return forward_.find(key) != forward_.end();
}

template <class Key, class Value>
bool invertible_map<Key, Value>::contains_value(const Value& value) const
{
    return backward_.find(value) != backward_.end();
}

template <class Key, class Value>
void invertible_map<Key, Value>::insert(const Key& key, const Value& value)
{
    forward_.insert(std::make_pair(key, value));
    backward_.insert(std::make_pair(value, key));
}

template <class Key, class Value>
void invertible_map<Key, Value>::insert(const std::pair<Key, Value>& pair)
{
    forward_.insert(pair);
    backward_.insert(std::make_pair(pair.second, pair.first));
}

template <class Key, class Value>
typename invertible_map<Key, Value>::const_iterator
invertible_map<Key, Value>::begin() const
{
    return Iterator{forward_.begin()};
}

template <class Key, class Value>
typename invertible_map<Key, Value>::const_iterator
invertible_map<Key, Value>::end() const
{
    return Iterator{forward_.end()};
}
}
}
