/**
 * @file functional.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_UTIL_FUNCTIONAL_H_
#define META_UTIL_FUNCTIONAL_H_

#include <functional>
#include <map>

namespace meta
{
namespace functional
{

/**
 * Memoizes a std::function.
 * @param fun the std::function to be memoized
 */
template <class Result, class... Args>
std::function<Result(Args...)> memoize(std::function<Result(Args...)> fun)
{
    return [fun](Args... args)
    {
        static std::map<std::tuple<Args...>, Result> map_;
        auto it = map_.find(std::make_tuple(args...));
        if (it != map_.end())
            return it->second;
        return map_[std::make_tuple(args...)] = fun(args...);
    };
}
}
}
#endif
