/**
 * @file invertible_map.tcc
 */

#include <sstream>
#include "io/parser.h"

namespace meta {
namespace util {

template <class Key, class Value>
invertible_map<Key, Value>::invertible_map() { /* nothing */ }

template <class Key, class Value>
bool invertible_map<Key, Value>::empty() const
{
    return _forward.empty();
}

template <class Key, class Value>
void invertible_map<Key, Value>::clear()
{
    _forward.clear();
    _backward.clear();
}

template <class Key, class Value>
size_t invertible_map<Key, Value>::size() const
{
    return _forward.size();
}

template <class Key, class Value>
Key invertible_map<Key, Value>::get_key(const Value & value) const
{
    auto it = _backward.find(value);
    if(it == _backward.end())
        return Key();
    return it->second;
}

template <class Key, class Value>
Value invertible_map<Key, Value>::get_value(const Key & key) const
{
    auto it = _forward.find(key);
    if(it == _forward.end())
        return Value{};
    return it->second;
}

template <class Key, class Value>
bool invertible_map<Key, Value>::contains_key(const Key & key) const
{
    return _forward.find(key) != _forward.end();
}

template <class Key, class Value>
bool invertible_map<Key, Value>::contains_value(const Value & value) const
{
    return _backward.find(value) != _backward.end();
}

template <class Key, class Value>
void invertible_map<Key, Value>::insert(const Key & key, const Value & value)
{
    _forward.insert(std::make_pair(key, value));
    _backward.insert(std::make_pair(value, key));
}

template <class Key, class Value>
void invertible_map<Key, Value>::insert(const std::pair<Key, Value> & pair)
{
    _forward.insert(pair);
    _backward.insert(std::make_pair(pair.second, pair.first));
}

template <class Key, class Value>
typename invertible_map<Key, Value>::const_iterator invertible_map<Key, Value>::begin() const
{
    return Iterator{_forward.begin()};
}

template <class Key, class Value>
typename invertible_map<Key, Value>::const_iterator invertible_map<Key, Value>::end() const
{
    return Iterator{_forward.end()};
}

}
}
