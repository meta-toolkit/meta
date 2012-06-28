/**
 * @file invertible_map.cpp
 */

template <class Key, class Value>
bool InvertibleMap<Key, Value>::empty() const
{
    return _forward.empty();
}

template <class Key, class Value>
size_t InvertibleMap<Key, Value>::size() const
{
    return _forward.size();
}

template <class Key, class Value>
Key InvertibleMap<Key, Value>::getKeyByValue(const Value & value) const
{
    auto it = _backward.find(value);
    if(it == _backward.end())
        return Key();
    return it->second;
}

template <class Key, class Value>
Value InvertibleMap<Key, Value>::getValueByKey(const Key & key) const
{
    auto it = _forward.find(key);
    if(it == _forward.end())
        return Value();
    return it->second;
}

template <class Key, class Value>
void InvertibleMap<Key, Value>::insert(const Key & key, const Value & value)
{
    _forward.insert(make_pair(key, value));
    _backward.insert(make_pair(value, key));
}

template <class Key, class Value>
map<Key, Value> InvertibleMap<Key, Value>::sortKeys() const
{
    map<Key, Value> sorted;
    for(auto & it: _forward)
        sorted.insert(it);
    return sorted;
}

template <class Key, class Value>
map<Value, Key> InvertibleMap<Key, Value>::sortValues() const
{
    map<Value, Key> sorted;
    for(auto & it: _backward)
        sorted.insert(it);
    return sorted;
}
