/**
 * @file traits.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_TRAITS_H_
#define META_UTIL_TRAITS_H_

#include <type_traits>

namespace meta
{
namespace util
{
template <class A, class B>
using disable_if_same_or_derived_t = typename std::
    enable_if<!std::is_base_of<A, typename std::remove_reference<B>::type>::
                  value>::type;

// @see http://stackoverflow.com/a/15394527
template <class T, class = void>
struct is_callable : std::is_function<T>
{
    // nothing
};

template <class T>
struct is_callable<T, typename std::
                          enable_if<std::is_same<decltype(void(&T::operator())),
                                                 void>::value>::type>
    : std::true_type
{
    // nothing
};
}
}
#endif
