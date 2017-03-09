/**
 * @file arena_allocator.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_ARENA_ALLOCATOR_H_
#define META_UTIL_ARENA_ALLOCATOR_H_

#include <memory>
#include <vector>

#include "meta/config.h"

namespace meta
{
namespace util
{

class arena_bad_alloc : public std::bad_alloc
{
  public:
    using std::bad_alloc::bad_alloc;
};

/**
 * Represents a fixed-size block of memory from which objects can be
 * allocated. Based on code written by Howard Hinnant and released under
 * the MIT license.
 * @see https://howardhinnant.github.io/stack_alloc.html
 */
template <std::size_t Alignment = alignof(std::max_align_t),
          class Allocator = std::allocator<char>>
class arena
{
  public:
    constexpr static std::size_t alignment = Alignment;

    arena(std::size_t bytes, const Allocator& alloc = Allocator{})
        : buffer_(bytes, char{0}, alloc), pos_{0}
    {
        buffer_.shrink_to_fit();
        // nothing
    }

    arena(const arena&) = delete;
    arena& operator=(const arena&) = delete;

    template <std::size_t TypeAlignment>
    char* allocate(std::size_t n)
    {
        static_assert(TypeAlignment <= alignment, "arena alignment too small");
        const auto aligned_n = align(n);
        if (buffer_.size() - pos_ < aligned_n)
            throw arena_bad_alloc{};

        char* r = &buffer_[pos_];
        pos_ += aligned_n;
        return r;
    }

    std::size_t size() const
    {
        return buffer_.size();
    }

    std::size_t used() const
    {
        return pos_;
    }

    void reset()
    {
        pos_ = 0;
    }

  private:
    static std::size_t align(std::size_t n)
    {
        return (n + (alignment - 1)) & ~(alignment - 1);
    }

    std::vector<char, Allocator> buffer_;
    std::size_t pos_;
};

/**
 * Allocator that places things within a specific arena. Based on code
 * written by Howard Hinnant and released under the MIT license.
 * @see https://howardhinnant.github.io/stack_alloc.html
 */
template <class T, class Arena = arena<>>
class arena_allocator
{
  public:
    using value_type = T;
    using arena_type = Arena;

    template <class U, class ArenaType>
    friend class arena_allocator;

    arena_allocator(const arena_allocator&) = default;
    arena_allocator& operator=(const arena_allocator&) = delete;

    arena_allocator(arena_type& arena) : arena_(arena)
    {
        // nothing
    }

    template <class U>
    arena_allocator(const arena_allocator<U, Arena>& alloc)
        : arena_(alloc.arena_)
    {
        // nothing
    }

    template <class U>
    struct rebind
    {
        using other = arena_allocator<U, Arena>;
    };

    T* allocate(std::size_t n)
    {
        return reinterpret_cast<T*>(
            arena_.template allocate<alignof(T)>(sizeof(T) * n));
    }

    void deallocate(T*, std::size_t)
    {
        // nothing
    }

    template <class T1, class A1, class T2, class A2>
    friend bool operator==(const arena_allocator<T1, A1>& lhs,
                           const arena_allocator<T2, A2>& rhs);

    template <class U, class A>
    friend class arena_allocator;

  private:
    arena_type& arena_;
};

template <class T1, class A1, class T2, class A2>
inline bool operator==(const arena_allocator<T1, A1>& lhs,
                       const arena_allocator<T2, A2>& rhs)
{
    return &lhs.arena_ == &rhs.arena_;
}

template <class T1, class A1, class T2, class A2>
inline bool operator!=(const arena_allocator<T1, A1>& lhs,
                       const arena_allocator<T2, A2>& rhs)
{
    return !(lhs == rhs);
}
}
}
#endif
