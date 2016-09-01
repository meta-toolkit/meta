/**
 * @file algorithm.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_ALGORITHM_H_
#define META_UTIL_ALGORITHM_H_

#include <algorithm>

#include "meta/config.h"

namespace meta
{
namespace util
{

/**
 * Applys the binary operator to each token in the range [first, last) that
 * is delimited a token in [s_first, s_last).
 *
 * @see http://tristanbrindle.com/posts/a-quicker-study-on-tokenising/
 */
template <class InputIt, class ForwardIt, class BinOp>
void for_each_token(InputIt first, InputIt last, ForwardIt s_first,
                    ForwardIt s_last, BinOp binary_op)
{
    while (first != last)
    {
        const auto pos = std::find_first_of(first, last, s_first, s_last);
        binary_op(first, pos);
        if (pos == last)
            break;
        first = std::next(pos);
    }
}

/**
 * Applys the binary operator to each token in the range [first, last) that
 * is delimited a token in delims.
 *
 * @see http://tristanbrindle.com/posts/a-quicker-study-on-tokenising/
 */
template <class InputIt, class Delims, class BinOp>
void for_each_token(InputIt first, InputIt last, const Delims& delims,
                    BinOp binary_op)
{
    for_each_token(first, last, std::begin(delims), std::end(delims),
                   binary_op);
}
}
}
#endif
