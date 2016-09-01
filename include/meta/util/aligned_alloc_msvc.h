/**
 * @file aligned_alloc_msvc.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 *
 * This file contains the implementation of aligned allocation/free for
 * Windows platforms where the C11 standard aligned_alloc is not available.
 */

#ifndef META_ALIGNED_ALLOC_MSVC_H_
#define META_ALIGNED_ALLOC_MSVC_H_

#include <cstddef>
#include <malloc.h>

#include "meta/config.h"

namespace meta
{
namespace util
{
namespace detail
{

inline void* aligned_alloc(std::size_t alignment, std::size_t size)
{
    // yes, windows is a special little snowflake; the parameter order
    // below is correct
    return ::_aligned_malloc(size, alignment);
}

inline void aligned_free(void* ptr)
{
    ::_aligned_free(ptr);
}
}
}
}
#endif
