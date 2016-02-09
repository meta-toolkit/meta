/**
 * @file vector.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_MATH_VECTOR_H_
#define META_MATH_VECTOR_H_

#include <cassert>
#include <algorithm>
#include <type_traits>
#include <vector>

#include "meta/util/array_view.h"

namespace meta
{
namespace math
{
namespace operators
{

template <class T, class Allocator = std::allocator<T>>
using vector = std::vector<T, Allocator>;

namespace detail
{
template <class T, class U>
struct common_type
{
    using type =
        typename std::remove_const<typename std::common_type<T, U>::type>::type;
    using vector_type = vector<type>;
};
}

// forward declarations: operator+

// - vector x vector
template <class T, class Allocator>
vector<T, Allocator> operator+(const vector<T, Allocator>& a,
                               const vector<T, Allocator>& b);

template <class T, class Allocator>
vector<T, Allocator> operator+(vector<T, Allocator>&& a,
                               const vector<T, Allocator>& b);

template <class T, class Allocator>
vector<T, Allocator> operator+(const vector<T, Allocator>& a,
                               vector<T, Allocator>&& b);

template <class T, class Allocator>
vector<T, Allocator> operator+(vector<T, Allocator>&& a,
                               vector<T, Allocator>&& b);

// - vector x array_view
template <class T, class U, class Allocator>
vector<T, Allocator> operator+(const vector<T, Allocator>& a,
                               util::array_view<U> b);

template <class T, class U, class Allocator>
vector<T, Allocator> operator+(vector<T, Allocator>&& a, util::array_view<U> b);

// - array_view x vector
template <class T, class U, class Allocator>
vector<T, Allocator> operator+(util::array_view<U> a,
                               const vector<T, Allocator>& b);

template <class T, class U, class Allocator>
vector<T, Allocator> operator+(util::array_view<U> a, vector<T, Allocator>&& b);

// - array_view x array_view
template <class T, class U>
typename detail::common_type<T, U>::vector_type
operator+(util::array_view<T> a, util::array_view<U> b);

// implementations: operator+

// - vector x vector
template <class T, class Allocator>
vector<T, Allocator> operator+(const vector<T, Allocator>& a,
                               const vector<T, Allocator>& b)
{
    vector<T, Allocator> result{a};
    return std::move(result) + util::array_view<const T>(b);
}

template <class T, class Allocator>
vector<T, Allocator> operator+(vector<T, Allocator>&& a,
                               const vector<T, Allocator>& b)
{
    return std::move(a) + util::array_view<const T>(b);
}

template <class T, class Allocator>
vector<T, Allocator> operator+(const vector<T, Allocator>& a,
                               vector<T, Allocator>&& b)
{
    return std::move(b) + util::array_view<const T>(a);
}

template <class T, class Allocator>
vector<T, Allocator> operator+(vector<T, Allocator>&& a,
                               vector<T, Allocator>&& b)
{
    return std::move(a) + util::array_view<const T>(b);
}

// - vector x array_view
template <class T, class U, class Allocator>
vector<T, Allocator> operator+(const vector<T, Allocator>& a,
                               util::array_view<U> b)
{
    vector<T, Allocator> result{a};
    return std::move(result) + b;
}

template <class T, class U, class Allocator>
vector<T, Allocator> operator+(vector<T, Allocator>&& a, util::array_view<U> b)
{
    vector<T, Allocator> result{std::move(a)};
    std::transform(b.begin(), b.end(), result.begin(), result.begin(),
                   [](const T& bval, const T& resval)
                   {
                       return bval + resval;
                   });
    return result;
}

// - array_view x vector
template <class T, class U, class Allocator>
vector<T, Allocator> operator+(util::array_view<U> a,
                               const vector<T, Allocator>& b)
{
    vector<T, Allocator> result{b};
    return std::move(result) + a;
}

template <class T, class U, class Allocator>
vector<T, Allocator> operator+(util::array_view<U> a, vector<T, Allocator>&& b)
{
    return std::move(b) + a;
}

// - array_view x array_view
template <class T, class U>
typename detail::common_type<T, U>::vector_type operator+(util::array_view<T> a,
                                                          util::array_view<U> b)
{
    typename detail::common_type<T, U>::vector_type result{a.begin(), a.end()};
    return std::move(result) + b;
}

// forward declarations: operator-

// - vector x vector
template <class T, class Allocator>
vector<T, Allocator> operator-(const vector<T, Allocator>& a,
                               const vector<T, Allocator>& b);

template <class T, class Allocator>
vector<T, Allocator> operator-(vector<T, Allocator>&& a,
                               const vector<T, Allocator>& b);

template <class T, class Allocator>
vector<T, Allocator> operator-(const vector<T, Allocator>& a,
                               vector<T, Allocator>&& b);

template <class T, class Allocator>
vector<T, Allocator> operator-(vector<T, Allocator>&& a,
                               vector<T, Allocator>&& b);

// - vector x array_view
template <class T, class U, class Allocator>
vector<T, Allocator> operator-(const vector<T, Allocator>& a,
                               util::array_view<U> b);

template <class T, class U, class Allocator>
vector<T, Allocator> operator-(vector<T, Allocator>&& a, util::array_view<U> b);

// - array_view x vector
template <class T, class U, class Allocator>
vector<T, Allocator> operator-(util::array_view<U> a,
                               const vector<T, Allocator>& b);

template <class T, class U, class Allocator>
vector<T, Allocator> operator-(util::array_view<U> a, vector<T, Allocator>&& b);

// - array_view x array_view
template <class T, class U>
typename detail::common_type<T, U>::vector_type
operator-(util::array_view<T> a, util::array_view<U> b);

// implementations: operator-

// vector x vector
template <class T, class Allocator>
vector<T, Allocator> operator-(const vector<T, Allocator>& a,
                               const vector<T, Allocator>& b)
{
    vector<T, Allocator> result{a};
    return std::move(result) - util::array_view<const T>(b);
}

template <class T, class Allocator>
vector<T, Allocator> operator-(vector<T, Allocator>&& a,
                               const vector<T, Allocator>& b)
{
    return std::move(a) - util::array_view<const T>(b);
}

template <class T, class Allocator>
vector<T, Allocator> operator-(const vector<T, Allocator>& a,
                               vector<T, Allocator>&& b)
{
    return util::array_view<const T>(a) - std::move(b);
}

template <class T, class Allocator>
vector<T, Allocator> operator-(vector<T, Allocator>&& a,
                               vector<T, Allocator>&& b)
{
    return std::move(a) - util::array_view<const T>(b);
}

// vector x array_view

template <class T, class U, class Allocator>
vector<T, Allocator> operator-(const vector<T, Allocator>& a,
                               util::array_view<U> b)
{
    vector<T, Allocator> result{std::move(a)};
    return std::move(result) - b;
}

template <class T, class U, class Allocator>
vector<T, Allocator> operator-(vector<T, Allocator>&& a, util::array_view<U> b)
{
    vector<T, Allocator> result{std::move(a)};
    std::transform(result.begin(), result.end(), b.begin(), result.begin(),
                   [](const T& resval, const T& bval)
                   {
                       return resval - bval;
                   });
    return result;
}

// array_view x vector
template <class T, class U, class Allocator>
vector<T, Allocator> operator-(util::array_view<U> a,
                               const vector<T, Allocator>& b)
{
    vector<T, Allocator> result{b};
    return a - std::move(result);
}

template <class T, class U, class Allocator>
vector<T, Allocator> operator-(util::array_view<U> a, vector<T, Allocator>&& b)
{
    vector<T, Allocator> result{std::move(b)};
    std::transform(a.begin(), a.end(), result.begin(), result.begin(),
                   [](const T& aval, const T& resval)
                   {
                       return aval - resval;
                   });
    return result;
}

template <class T, class U>
typename detail::common_type<T, U>::vector_type operator-(util::array_view<T> a,
                                                          util::array_view<U> b)
{
    typename detail::common_type<T, U>::vector_type result{a.begin(), a.end()};
    return std::move(result) - b;
}
}
}
}
#endif
