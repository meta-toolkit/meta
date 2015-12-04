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

#if META_HAS_ALIGNED_ALLOC
#include "util/aligned_alloc.h"
#elif META_HAS_POSIX_MEMALIGN
#include "util/aligned_alloc_posix.h"
#elif META_HAS_ALIGNED_MALLOC
#include "util/aligned_alloc_msvc.h"
#endif

namespace meta
{
namespace util
{

namespace detail
{
template <class Integer>
Integer idiv_ceil(Integer num, Integer denom)
{
    // this should be 1 instruction on most architectures since the div
    // instruction also returns the remainder
    return (num / denom) + (num % denom != 0);
}
}

template <class T, std::size_t Alignment>
struct aligned_allocator
{
    using value_type = T;

    const static constexpr std::size_t alignment_size
        = (Alignment > alignof(T)) ? Alignment : alignof(T);

    aligned_allocator() = default;

    template <class U, std::size_t OtherAlign>
    aligned_allocator(const aligned_allocator<U, OtherAlign>&)
    {
        // nothing
    }

    T* allocate(std::size_t n)
    {
        // determine adjusted size
        // ::aligned_alloc requires the size to be an integer multiple of
        // the requested alignment
        auto size
            = alignment_size * detail::idiv_ceil(n * sizeof(T), alignment_size);
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

template <class T, std::size_t TAlignment, class U, std::size_t UAlignment>
bool operator==(const aligned_allocator<T, TAlignment>&,
                const aligned_allocator<U, UAlignment>&)
{
    return true;
}

template <class T, std::size_t TAlignment, class U, std::size_t UAlignment>
bool operator!=(const aligned_allocator<T, TAlignment>&,
                const aligned_allocator<U, UAlignment>&)
{
    return false;
}

template <class T, std::size_t Alignment = 64>
using aligned_vector = std::vector<T, aligned_allocator<T, Alignment>>;
}
}
#endif
