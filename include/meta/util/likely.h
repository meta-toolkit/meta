/**
 * @file likely.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_LIKELY_H_
#define META_UTIL_LIKELY_H_

#include "meta/config.h"

#if META_HAS_BUILTIN_EXPECT
#define META_LIKELY(x) __builtin_expect(!!(x), 1)
#define META_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define META_LIKELY(x) x
#define META_UNLIKELY(x) x
#endif

#endif
