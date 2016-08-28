/**
 * @file sparse_vector.tcc
 * @author Chase Geigle
 */

#include <algorithm>

#include "meta/util/sparse_vector.h"

namespace meta
{
namespace util
{

template <class Index, class Value>
sparse_vector<Index, Value>::sparse_vector(uint64_t size) : storage_(size)
{
    // nothing
}

template <class Index, class Value>
template <class Iter>
sparse_vector<Index, Value>::sparse_vector(Iter begin, Iter end)
    : storage_{begin, end}
{
    // nothing
}

template <class Index, class Value>
Value& sparse_vector<Index, Value>::operator[](const Index& index)
{
    auto it = std::lower_bound(
        std::begin(storage_), std::end(storage_), index,
        [](const pair_type& p, const Index& idx) { return p.first < idx; });

    if (it == std::end(storage_))
    {
        storage_.emplace_back(index, Value{});
        return storage_.back().second;
    }
    else if (it->first != index)
    {
        auto ins = storage_.emplace(it, index, Value{});
        return ins->second;
    }
    else
    {
        return it->second;
    }
}

template <class Index, class Value>
Value sparse_vector<Index, Value>::at(const Index& index) const
{
    auto it = std::lower_bound(
        std::begin(storage_), std::end(storage_), index,
        [](const pair_type& p, const Index& idx) { return p.first < idx; });

    if (it == std::end(storage_) || it->first != index)
        return Value{};
    return it->second;
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::find(const Index& index) const
    -> const_iterator
{
    auto it = std::lower_bound(
        std::begin(storage_), std::end(storage_), index,
        [](const pair_type& p, const Index& idx) { return p.first < idx; });

    if (it == std::end(storage_) || it->first != index)
        return std::end(storage_);

    return it;
}

template <class Index, class Value>
template <class... Ts>
void sparse_vector<Index, Value>::emplace_back(Ts&&... ts)
{
    storage_.emplace_back(std::forward<Ts>(ts)...);
}

template <class Index, class Value>
void sparse_vector<Index, Value>::reserve(uint64_t size)
{
    storage_.reserve(size);
}

template <class Index, class Value>
void sparse_vector<Index, Value>::clear()
{
    storage_.clear();
}

template <class Index, class Value>
void sparse_vector<Index, Value>::shrink_to_fit()
{
    storage_.shrink_to_fit();
}

template <class Index, class Value>
void sparse_vector<Index, Value>::condense()
{
    Value default_value{};

    // erase-remove idiom looking for value-initalized elements
    storage_.erase(std::remove_if(storage_.begin(), storage_.end(),
                                  [&](const pair_type& p) {
                                      return p.second == default_value;
                                  }),
                   storage_.end());
    shrink_to_fit();
}

template <class Index, class Value>
uint64_t sparse_vector<Index, Value>::size() const
{
    return storage_.size();
}

template <class Index, class Value>
uint64_t sparse_vector<Index, Value>::capacity() const
{
    return storage_.capacity();
}

template <class Index, class Value>
bool sparse_vector<Index, Value>::empty() const
{
    return storage_.empty();
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::contents() const -> const container_type&
{
    return storage_;
}

template <class Index, class Value>
void sparse_vector<Index, Value>::contents(container_type cont)
{
    storage_ = std::move(cont);
    std::sort(std::begin(storage_), std::end(storage_),
              [](const pair_type& a, const pair_type& b) {
                  return a.first < b.first;
              });
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::begin() -> iterator
{
    return std::begin(storage_);
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::begin() const -> const_iterator
{
    return std::begin(storage_);
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::cbegin() const -> const_iterator
{
    return storage_.cbegin();
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::end() -> iterator
{
    return std::end(storage_);
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::end() const -> const_iterator
{
    return std::end(storage_);
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::cend() const -> const_iterator
{
    return storage_.cend();
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::erase(iterator pos) -> iterator
{
    return storage_.erase(pos);
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::erase(iterator first, iterator last)
    -> iterator
{
    return storage_.erase(first, last);
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::operator+=(const sparse_vector& rhs)
    -> sparse_vector&
{
    // use index into the current vector so that we can properly handle
    // invalidation of iterators during the loop
    uint64_t idx = 0;
    uint64_t end = size();

    auto second_it = rhs.begin();
    auto second_end = rhs.end();

    while (idx != end && second_it != second_end)
    {
        if (storage_[idx].first == second_it->first)
        {
            storage_[idx].second += second_it->second;
            ++idx;
            ++second_it;
        }
        else if (storage_[idx].first < second_it->first)
        {
            ++idx;
        }
        else
        {
            using diff_type = typename iterator::difference_type;
            storage_.emplace(begin() + static_cast<diff_type>(idx),
                             second_it->first, second_it->second);
            ++idx;
            ++second_it;
        }
    }

    for (; second_it != second_end; ++second_it)
    {
        storage_.emplace_back(second_it->first, second_it->second);
    }

    return *this;
}

template <class Index, class Value>
auto sparse_vector<Index, Value>::operator-=(const sparse_vector& rhs)
    -> sparse_vector&
{
    // use index into the current vector so that we can properly handle
    // invalidation of iterators during the loop
    uint64_t idx = 0;
    uint64_t end = size();

    auto second_it = rhs.begin();
    auto second_end = rhs.end();

    while (idx != end && second_it != second_end)
    {
        if (storage_[idx].first == second_it->first)
        {
            storage_[idx].second -= second_it->second;
            ++idx;
            ++second_it;
        }
        else if (storage_[idx].first < second_it->first)
        {
            ++idx;
        }
        else
        {
            using diff_type = typename iterator::difference_type;
            storage_.emplace(begin() + static_cast<diff_type>(idx),
                             second_it->first, -second_it->second);
            ++idx;
            ++second_it;
        }
    }

    for (; second_it != second_end; ++second_it)
    {
        storage_.emplace_back(second_it->first, -second_it->second);
    }

    return *this;
}
}
}
