/**
 * @file common.tcc
 */

#include <sstream>

namespace meta {

template <class T>
std::string common::toString(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

template <class Key, class Value, class Hash>
Value common::safe_at(const std::unordered_map<Key, Value, Hash> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

template <class Key, class Value>
Value common::safe_at(const std::unordered_map<Key, Value> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

}
