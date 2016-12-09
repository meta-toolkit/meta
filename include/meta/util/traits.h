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
}
}
#endif
