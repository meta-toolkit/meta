/**
 * @file persistent_stack.tcc
 * @author Chase Geigle
 */

#include "meta/util/persistent_stack.h"

namespace meta
{
namespace util
{

template <class T>
persistent_stack<T>::node::node(T item, std::shared_ptr<node> previous)
    : data{std::move(item)}, prev{std::move(previous)}
{
    // nothing
}

template <class T>
persistent_stack<T>::persistent_stack()
    : head_{nullptr}, size_{0}
{
    // nothing
}

template <class T>
persistent_stack<T> persistent_stack<T>::push(T data) const
{
    auto n = std::make_shared<node>(std::move(data), head_);
    return {n, size_ + 1};
}

template <class T>
persistent_stack<T> persistent_stack<T>::pop() const
{
    if (size_ == 0)
        throw persistent_stack_exception{"pop() called on empty stack"};

    return {head_->prev, size_ - 1};
}

template <class T>
const T& persistent_stack<T>::peek() const
{
    if (!head_)
        throw persistent_stack_exception{"peek() called on empty stack"};

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
