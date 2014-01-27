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
