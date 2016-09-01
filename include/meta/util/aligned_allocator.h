/**
 * @file aligned_allocator.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_ALIGNED_ALLOCATOR_H_
#define META_UTIL_ALIGNED_ALLOCATOR_H_

#include <cstddef>
#include <new>
#include <vector>

#include "meta/config.h"

#if META_HAS_ALIGNED_ALLOC
#include "meta/util/aligned_alloc.h"
#elif META_HAS_POSIX_MEMALIGN
#include "meta/util/aligned_alloc_posix.h"
#elif META_HAS_ALIGNED_MALLOC
#include "meta/util/aligned_alloc_msvc.h"
#endif
#include "meta/math/integer.h"

namespace meta
{
namespace util
{

template <class T, std::size_t Alignment>
struct aligned_allocator
{
    using value_type = T;

    aligned_allocator() = default;

    template <class U>
    aligned_allocator(const aligned_allocator<U, Alignment>&)
    {
        // nothing
    }

    template <class U>
    struct rebind
    {
        using other = aligned_allocator<U, Alignment>;
    };

    T* allocate(std::size_t n)
    {
        const static constexpr std::size_t alignment_size
            = (Alignment > alignof(T)) ? Alignment : alignof(T);
        // determine adjusted size
        // ::aligned_alloc requires the size to be an integer multiple of
        // the requested alignment
        auto size = alignment_size
                    * math::integer::div_ceil(n * sizeof(T), alignment_size);
        auto ptr = static_cast<T*>(detail::aligned_alloc(alignment_size, size));

        if (!ptr && n > 0)
            throw std::bad_alloc{};
        return ptr;
    }

    void deallocate(T* p, std::size_t)
    {
        detail::aligned_free(p);
    }
};

template <class T, class U, std::size_t Alignment>
bool operator==(const aligned_allocator<T, Alignment>&,
                const aligned_allocator<U, Alignment>&)
{
    return true;
}

template <class T, class U, std::size_t Alignment>
bool operator!=(const aligned_allocator<T, Alignment>&,
                const aligned_allocator<U, Alignment>&)
{
    return false;
}

template <class T, std::size_t Alignment = 64>
using aligned_vector = std::vector<T, aligned_allocator<T, Alignment>>;
}
}
#endif
