/**
 * @file mapping.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_MAPPING_H_
#define META_UTIL_MAPPING_H_

#include <fstream>
#include <string>
#include <vector>

#include "meta/config.h"
#include "meta/util/invertible_map.h"

namespace meta
{
namespace map
{

/**
 * This safe_at allows the use of a hash function to be specified.
 * @param map The map to search
 * @param key The key in the map to search for
 * @return the value for a given key in map if it exists; otherwise,
 * the default Value() is returned.
 */
template <class Key, class Value, class... Args,
          template <class, class, class...> class Map>
Value safe_at(const Map<Key, Value, Args...>& map, const Key& key)
{
    auto it = map.find(key);
    if (it == map.end())
        return Value{};
    return it->second;
}

/**
 * Saves any arbitrary mapping to the disk.
 * @param map The map to read key, value pairs from
 * @param filename The name to save the mapping as
 */
template <class Key, class Value>
void save_mapping(const util::invertible_map<Key, Value>& map,
                  const std::string& filename)
{
    std::ofstream outfile{filename};
    for (auto& p : map)
        outfile << p.first << " " << p.second << "\n";
}

/**
 * Vector-specific version of save_mapping: saves any arbitrary mapping
 * to the disk.
 * @param map The map to read key, value pairs from
 * @param filename The name to save the mapping as
 */
template <class T>
void save_mapping(const std::vector<T>& vec, const std::string& filename)
{
    std::ofstream outfile{filename};
    for (auto& v : vec)
        outfile << v << "\n";
}

/**
 * @param map The map to load information into
 * @param filename The file containing key, value pairs
 */
template <class Key, class Value>
void load_mapping(util::invertible_map<Key, Value>& map,
                  const std::string& filename)
{
    std::ifstream input{filename};
    Key k;
    Value v;
    while ((input >> k) && (input >> v))
        map.insert(std::make_pair(k, v));
}

/**
 * Vector-specific version of load_mapping.
 * @param vec The vector to load information into
 * @param filename The file containing the vector's data.
 */
template <class T>
void load_mapping(std::vector<T>& vec, const std::string& filename)
{
    std::ifstream input{filename};
    T val;
    while (input >> val)
        vec.push_back(val);
}
}
}

#endif
