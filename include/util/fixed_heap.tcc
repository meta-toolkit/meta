/**
 * @file fixed_heap.tcc
 * @author Sean Massung
 */

#include <cassert>
#include <algorithm>

namespace meta
{
namespace util
{
template <class T, class Comp>
fixed_heap<T, Comp>::fixed_heap(uint64_t max_elems, Comp comp)
    : max_elems_{max_elems}, comp_(comp)
{
    // nothing
}

template <class T, class Comp>
template <class... Args>
void fixed_heap<T, Comp>::emplace(Args&&... args)
{
    pq_.emplace_back(std::forward<Args>(args)...);
    std::push_heap(pq_.begin(), pq_.end(), comp_);
    if (size() > max_elems())
    {
        std::pop_heap(pq_.begin(), pq_.end(), comp_);
        pq_.pop_back();
    }
}

template <class T, class Comp>
void fixed_heap<T, Comp>::push(const T& elem)
{
    pq_.push_back(elem);
    std::push_heap(pq_.begin(), pq_.end(), comp_);
    if (size() > max_elems())
    {
        std::pop_heap(pq_.begin(), pq_.end(), comp_);
        pq_.pop_back();
    }
}

template <class T, class Comp>
auto fixed_heap<T, Comp>::size() const -> size_type
{
    return pq_.size();
}

template <class T, class Comp>
auto fixed_heap<T, Comp>::max_elems() const -> size_type
{
    return max_elems_;
}

template <class T, class Comp>
std::vector<T> fixed_heap<T, Comp>::extract_top()
{
    using diff_type = typename decltype(pq_.begin())::difference_type;
    for (size_type i = 0; i < pq_.size(); ++i)
        std::pop_heap(pq_.begin(), pq_.end() - static_cast<diff_type>(i),
                      comp_);

    std::vector<T> result = std::move(pq_);
    assert(pq_.empty());
    return result;
}

template <class T, class Comp>
typename fixed_heap<T, Comp>::const_iterator fixed_heap<T, Comp>::begin() const
{
    return pq_.cbegin();
}

template <class T, class Comp>
typename fixed_heap<T, Comp>::const_iterator fixed_heap<T, Comp>::end() const
{
    return pq_.cend();
}
}
}
