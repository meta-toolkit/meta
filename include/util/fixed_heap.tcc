/**
 * @file fixed_heap.tcc
 * @author Sean Massung
 */

namespace meta
{
namespace util
{
template <class T, class Comp>
fixed_heap<T, Comp>::fixed_heap(uint64_t max_elems, Comp comp)
    : max_elems_{max_elems}, comp_{comp}, pq_{comp}
{
    // nothing
}

template <class T, class Comp>
template <class... Args>
void fixed_heap<T, Comp>::emplace(Args&&... args)
{
    pq_.emplace(std::forward<Args>(args)...);
    if (size() > max_elems())
        pq_.pop();
}

template <class T, class Comp>
void fixed_heap<T, Comp>::push(const T& elem)
{
    pq_.push(elem);
    if (size() > max_elems())
        pq_.pop();
}

template <class T, class Comp>
uint64_t fixed_heap<T, Comp>::size() const
{
    return pq_.size();
}

template <class T, class Comp>
uint64_t fixed_heap<T, Comp>::max_elems() const
{
    return max_elems_;
}

template <class T, class Comp>
std::vector<T> fixed_heap<T, Comp>::reverse_and_clear()
{
    std::vector<T> sorted;
    sorted.reserve(size());
    while (!pq_.empty())
    {
        sorted.emplace_back(std::move(pq_.top()));
        pq_.pop();
    }
    std::reverse(sorted.begin(), sorted.end());
    return sorted;
}
}
}
