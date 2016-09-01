/**
 * @file aligned_alloc_posix.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 *
 * This file contains the implementation of aligned allocation/free for
 * platforms that support posix_memalign but not the C11 standard
 * aligned_alloc.
 */

#ifndef META_ALIGNED_ALLOC_POSIX_H_
#define META_ALIGNED_ALLOC_POSIX_H_

#include <cstdlib>

#include "meta/config.h"

namespace meta
{
namespace util
{
namespace detail
{

inline void* aligned_alloc(std::size_t alignment, std::size_t size)
{
    void* ptr;
    if (::posix_memalign(&ptr, alignment, size))
        return nullptr;
    return ptr;
}

inline void aligned_free(void* ptr)
{
    ::free(ptr);
}
}
}
}
#endif
