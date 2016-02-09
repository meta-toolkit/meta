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
        return end_ - start_;
    }

  private:
    T* start_;
    T* end_;
};
}
}
#endif
