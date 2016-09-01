/**
 * @file integer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_MATH_INTEGER_H_
#define META_MATH_INTEGER_H_

#include "meta/config.h"

namespace meta
{
namespace math
{
namespace integer
{
/**
 * @param num The numerator
 * @param denom The denominator
 * @return \f$\lceil \frac{num}{denom} \rceil\f$
 */
template <class IntType1, class IntType2>
auto div_ceil(IntType1 num, IntType2 denom) -> decltype(num / denom)
{
    auto denominator = static_cast<decltype(num / denom)>(denom);
    // this should be 1 instruction on most architectures since the div
    // instruction also returns the remainder
    return (num / denominator) + (num % denominator != 0);
}
}
}
}
#endif
