/**
 * @file persistent_stack.tcc
 * @author Chase Geigle
 */

#include "util/persistent_stack.h"

namespace meta
{
namespace util
{

template <class T>
persistent_stack<T>::persistent_stack()
    : head_{nullptr}, size_{0}
{
    // nothing
}

template <class T>
template <class D>
persistent_stack<T> persistent_stack<T>::push(D&& data) const
{
    auto n = std::make_shared<node>(std::forward<D>(data), head_);
    return {n, size_ + 1};
}

template <class T>
persistent_stack<T> persistent_stack<T>::pop() const
{
    if (size_ == 0)
        throw exception{"pop() called on empty stack"};

    return {head_->prev, size_ - 1};
}

template <class T>
const T& persistent_stack<T>::peek() const
{
    return head_->data;
}

template <class T>
uint64_t persistent_stack<T>::size() const
{
    return size_;
}

template <class T>
persistent_stack<T>::persistent_stack(std::shared_ptr<node> head, uint64_t size)
    : head_{std::move(head)}, size_{size}
{
    // nothing
}
}
}
