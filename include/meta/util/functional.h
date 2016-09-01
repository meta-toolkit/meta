/**
 * @file functional.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_FUNCTIONAL_H_
#define META_UTIL_FUNCTIONAL_H_

#include <algorithm>
#include <functional>
#include <map>

#include "meta/config.h"

namespace meta
{
namespace functional
{

/**
 * Memoizes a std::function.
 * @param fun The std::function to be memoized
 * @return a memoized version of the parameter
 */
template <class Result, class... Args>
std::function<Result(Args...)> memoize(std::function<Result(Args...)> fun)
{
    return [fun](Args... args) {
        static std::map<std::tuple<Args...>, Result> map_;
        auto it = map_.find(std::make_tuple(args...));
        if (it != map_.end())
            return it->second;
        return map_[std::make_tuple(args...)] = fun(args...);
    };
}

/**
 * Obtains the argument that maximizes the output of a function on a
 * sequence.
 *
 * @param begin The beginning iterator
 * @param end The ending iterator
 * @param fn The function to apply to each element
 */
template <class Iter, class Function>
Iter argmax(Iter begin, Iter end, Function&& fn)
{
    using T = decltype(*begin);
    return std::max_element(
        begin, end, [&](const T& a, const T& b) { return fn(a) < fn(b); });
}
}
}
#endif
