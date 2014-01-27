/**
 * @file common.h
 * Includes code shared by many classes.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdint>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>

#include "logging/logger.h"
#include "util/invertible_map.h"

namespace meta {
namespace common {

/**
 * Times a given function.
 * @param functor the function to be timed
 * @return the length of time, expressed as a Duration, the function
 *  took to run. Defaults to milliseconds.
 */
template <class Duration = std::chrono::milliseconds, class Functor>
Duration time(Functor && functor)
{
    auto start = std::chrono::steady_clock::now();
    functor();
    auto end   = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<Duration>(end - start);
}

/**
 * This safe_at allows the use of a hash function to be specified.
 * @return the value for a given key in map if it exists; otherwise,
 * the default Value() is returned.
 */
template <class Key,
            class Value,
            class... Args,
            template <class, class, class...> class Map> 
Value safe_at(const Map<Key, Value, Args...> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

/**
 * Memoizes a std::function.
 * @param fun the std::function to be memoized
 */
template <class Result, class... Args>
std::function<Result(Args...)> memoize(std::function<Result(Args...)> fun)
{
    return [fun](Args... args) {
        static std::map<std::tuple<Args...>, Result> map_;
        auto it = map_.find(std::make_tuple(args...));
        if(it != map_.end())
            return it->second;
        return map_[std::make_tuple(args...)] = fun(args...);
    };
}

/**
 * Saves any arbitrary mapping to the disk.
 * @param map The map to read key, value pairs from
 * @param filename The name to save the mapping as
 */
template <class Key, class Value>
void save_mapping(const util::invertible_map<Key, Value> & map,
                    const std::string & filename)
{
    std::ofstream outfile{filename};
    for(auto & p: map)
        outfile << p.first << " " << p.second << "\n";
}

/**
 * Vector-specific version of save_mapping: saves any arbitrary mapping
 * to the disk.
 * @param map The map to read key, value pairs from
 * @param filename The name to save the mapping as
 */
template <class T>
void save_mapping(const std::vector<T> & vec,
                    const std::string & filename)
{
    std::ofstream outfile{filename};
    for(auto & v: vec)
        outfile << v << "\n";
}

/**
 * @param map The map to load information into
 * @param filename The file containing key, value pairs
 */
template <class Key, class Value>
void load_mapping(util::invertible_map<Key, Value> & map,
                    const std::string & filename)
{
    std::ifstream input{filename};
    Key k;
    Value v;
    while((input >> k) && (input >> v))
        map.insert(std::make_pair(k, v));
}

/**
 * Vector-specific version of load_mapping.
 * @param vec The vector to load information into
 * @param filename The file containing the vector's data.
 */
template <class T>
void load_mapping(std::vector<T> & vec, const std::string & filename)
{
    std::ifstream input{filename};
    T val;
    while(input >> val)
        vec.push_back(val);
}

/**
 * Constructs a unique ptr in place.
 * @param args The parameters to the constructor
 */
template <class T, class... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>{new T{std::forward<Args>(args)...}};
}

}
}

#endif
