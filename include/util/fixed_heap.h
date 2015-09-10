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

#include <queue>

namespace meta
{
namespace util
{
/**
 * Keeps a constant number of high-priority elements. This is useful for finding
 * the "top-k" T elements using the comparison function Comp.
 */
template <class T, class Comp>
class fixed_heap
{
  public:
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
    uint64_t size() const;

    /**
     * @return the maximum number of elements this heap will store
     */
    uint64_t max_elems() const;

    /**
     * @return a reverse-sorted list
     */
    std::vector<T> reverse_and_clear();

  private:
    uint64_t max_elems_;
    Comp comp_;
    std::priority_queue<T, std::vector<T>, decltype(comp_)> pq_;
};
}
}

#include "fixed_heap.tcc"
#endif
