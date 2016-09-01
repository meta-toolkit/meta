/**
 * @file fixed_heap.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FIXED_HEAP_H_
#define META_FIXED_HEAP_H_

#include <vector>

#include "meta/config.h"

namespace meta
{
namespace util
{
/**
 * Keeps a constant number of high-priority elements. This is useful for finding
 * the "top-k" T elements using the comparison function Comp.
 *
 * Internally, this class maintains a min-heap of max size `max_elems`,
 * meaning that a push/emplace takes \f$O(\log k)\f$ where \f$k\f$ is
 * `max_elems` passed on construction.
 *
 * Your comparison function should be the same as you would pass to e.g.
 * std::sort such that the result would be the elements in **descending**
 * order.
 */
template <class T, class Comp>
class fixed_heap
{
  public:
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using size_type = typename std::vector<T>::size_type;

    /**
     * @param max_elems
     * @param comp The priority comparison function for elements in this heap
     */
    fixed_heap(uint64_t max_elems, Comp comp);

    /**
     * @param elem The element to insert; it may or may not be inserted
     * depending on the size and priority of other elements in the heap
     */
    void push(const T& elem);

    /**
     * @param elem The element to emplace; it may or may not be inserted
     * depending on the size and priority of other elements in the heap
     */
    template <class... Args>
    void emplace(Args&&... args);

    /**
     * @return the current number of elements in this heap; will always be less
     * than or equal to max_elems()
     */
    size_type size() const;

    /**
     * @return the maximum number of elements this heap will store
     */
    size_type max_elems() const;

    /**
     * Clears the heap and returns the top elements
     * @return the top elements in sorted order
     */
    std::vector<T> extract_top();

    /**
     * @return a const_iterator to the beginning of the fixed_heap
     * @note the heap is not fully sorted
     */
    const_iterator begin() const;

    /**
     * @return a const_iterator to the end of the fixed_heap
     * @note the heap is not fully sorted
     */
    const_iterator end() const;

  private:
    uint64_t max_elems_;
    Comp comp_;
    std::vector<T> pq_;
};
}
}

#include "meta/util/fixed_heap.tcc"
#endif
