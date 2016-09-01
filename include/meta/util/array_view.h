/**
 * @file array_view.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_ARRAY_VIEW_H_
#define META_UTIL_ARRAY_VIEW_H_

#include <cstddef>
#include <vector>

#include "meta/config.h"

namespace meta
{
namespace util
{

/**
 * A non-owning reference to an array (or part of one). The underlying data
 * must outlive the array_view on top of it.
 */
template <class T>
class array_view
{
  public:
    /**
     * Constructs an empty array view.
     */
    array_view() : start_{nullptr}, end_{nullptr}
    {
        // nothing
    }

    /**
     * Constructs an array view starting at the given starting point of
     * the specified length.
     *
     * @param start The start point
     * @param len The length of the array_view
     */
    array_view(T* start, std::size_t len) : start_{start}, end_{start + len}
    {
        // nothing
    }

    /**
     * Constructs an array view starting at the given starting point and
     * ending at the given ending point (exclusive).
     *
     * @param start The starting point
     * @param end The ending point
     */
    array_view(T* start, T* end) : start_{start}, end_{end}
    {
        // nothing
    }

    /**
     * Constructs an array_view over a std::vector.
     */
    template <class U, class Allocator>
    array_view(const std::vector<U, Allocator>& container)
        : array_view(container.data(), container.size())
    {
        // nothing
    }

    /**
     * Constructs an array_view over a std::vector.
     */
    template <class U, class Allocator>
    array_view(std::vector<U, Allocator>& container)
        : array_view(container.data(), container.size())
    {
        // nothing
    }

    /**
     * Constructs an array_view from a compatible other array_view.
     */
    template <class U, class = typename std::
                           enable_if<std::is_convertible<U, T>::value>::type>
    array_view(const array_view<U>& av) : start_{av.begin()}, end_{av.end()}
    {
        // nothing
    }

    /**
     * @return an iterator to the start
     */
    T* begin() const
    {
        return start_;
    }

    /**
     * @return an iterator to the end
     */
    T* end() const
    {
        return end_;
    }

    /**
     * @param idx The index to access
     * @return the element at that position
     */
    const T& operator[](std::size_t idx) const
    {
        return begin()[idx];
    }

    /**
     * @param idx The index to access
     * @return the element at that position
     */
    T& operator[](std::size_t idx)
    {
        return begin()[idx];
    }

    /**
     * @return the number of elements in this array_view
     */
    std::size_t size() const
    {
        return static_cast<std::size_t>(end_ - start_);
    }

  private:
    T* start_;
    T* end_;
};
}
}
#endif
