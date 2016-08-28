/**
 * @file optional.tcc
 * @author Chase Geigle
 */

#include "meta/util/optional.h"

namespace meta
{
namespace util
{

template <class T>
optional_storage<T>::optional_storage(trivial_init_t)
    : dummy_{}
{
    /* nothing */
}

template <class T>
template <class... Args>
optional_storage<T>::optional_storage(Args&&... args)
    : value_(std::forward<Args>(args)...)
{
    /* nothing */
}

template <class T>
optional<T>::optional()
    : initialized_{false}, storage_{trivial_init}
{
    /* nothing */
}

template <class T>
optional<T>::optional(nullopt_t)
    : optional()
{
    /* nothing */
}

template <class T>
optional<T>::optional(const T& value)
    : initialized_{true}, storage_{value}
{
    /* nothing */
}

template <class T>
optional<T>::optional(T&& value)
    : initialized_{true}, storage_{std::move(value)}
{
    /* nothing */
}

template <class T>
optional<T>::optional(const optional& opt)
    : initialized_{opt.initialized_}, storage_{trivial_init}
{
    if (opt.initialized_)
        new (dataptr()) T{*opt}; // placement new
}

template <class T>
optional<T>::optional(optional&& opt)
    : initialized_{opt.initialized_}, storage_{trivial_init}
{
    if (opt.initialized_)
        new (dataptr()) T(std::move(opt.storage_.value_));
}

template <class T>
optional<T>& optional<T>::operator=(optional<T> rhs)
{
    swap(rhs);
    return *this;
}

template <class T>
void optional<T>::swap(optional<T>& other)
{
    if (initialized_ && !other.initialized_)
    {
        clear();
    }
    else if (!initialized_ && other.initialized_)
    {
        initialized_ = true;
        new (dataptr()) T{std::move(*other)};
        other.clear();
    }
    else if (initialized_ && other.initialized_)
    {
        std::swap(storage_.value_, other.storage_.value_);
    }
}

template <class T>
optional<T>::~optional()
{
    clear();
}

template <class T>
const T& optional<T>::operator*() const
{
    if (!initialized_)
        throw bad_optional_access{"access attempted on uninitialized option"};
    return storage_.value_;
}

template <class T>
T& optional<T>::operator*()
{
    if (!initialized_)
        throw bad_optional_access{"access attempted on uninitialized option"};
    return storage_.value_;
}

template <class T>
const T* optional<T>::operator->() const
{
    if (!initialized_)
        throw bad_optional_access{"access attempted on uninitialized option"};
    return dataptr();
}

template <class T>
T* optional<T>::operator->()
{
    if (!initialized_)
        throw bad_optional_access{"access attempted on uninitialized option"};
    return dataptr();
}

template <class T>
optional<T>::operator bool() const
{
    return initialized_;
}

template <class T>
void optional<T>::clear()
{
    if (initialized_)
        storage_.value_.T::~T(); // yes, this is real life
    initialized_ = false;
}

template <class T>
template <class U>
T optional<T>::value_or(U&& default_value) const &
{
    return bool(*this) ? **this
                       : static_cast<T>(std::forward<U>(default_value));
}

template <class T>
template <class U>
T optional<T>::value_or(U&& default_value) &&
{
    return bool(*this) ? std::move(**this)
                       : static_cast<T>(std::forward<U>(default_value));
}

template <class T>
const T* optional<T>::dataptr() const
{
    return std::addressof(storage_.value_);
}

template <class T>
T* optional<T>::dataptr()
{
    return std::addressof(storage_.value_);
}

template <class T>
bool operator<(const optional<T>& lhs, const optional<T>& rhs)
{
    if (lhs && rhs)
        return *lhs < *rhs;

    if (!lhs)
        return false;

    return true;
}
}
}
