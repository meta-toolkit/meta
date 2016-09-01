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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <type_traits>
#include <vector>

#include "meta/config.h"
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
    std::transform(
        b.begin(), b.end(), result.begin(), result.begin(),
        [](const T& bval, const T& resval) { return bval + resval; });
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
    std::transform(
        result.begin(), result.end(), b.begin(), result.begin(),
        [](const T& resval, const T& bval) { return resval - bval; });
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
    std::transform(
        a.begin(), a.end(), result.begin(), result.begin(),
        [](const T& aval, const T& resval) { return aval - resval; });
    return result;
}

template <class T, class U>
typename detail::common_type<T, U>::vector_type operator-(util::array_view<T> a,
                                                          util::array_view<U> b)
{
    typename detail::common_type<T, U>::vector_type result{a.begin(), a.end()};
    return std::move(result) - b;
}

// operator/
template <class T, class Allocator, class U>
vector<T, Allocator> operator/(vector<T, Allocator>&& vec, U denom)
{
    vector<T> result{std::move(vec)};
    std::transform(result.begin(), result.end(), result.begin(),
                   [=](const T& elem) { return elem / denom; });
    return result;
}

template <class T, class Allocator, class U>
vector<T, Allocator> operator/(const vector<T, Allocator>& vec, U denom)
{
    vector<T, Allocator> result{vec};
    return std::move(result) / denom;
}

template <class T, class U>
vector<typename std::remove_const<T>::type> operator/(util::array_view<T> vec,
                                                      U denom)
{
    vector<typename std::remove_const<T>::type> result{vec.begin(), vec.end()};
    return std::move(result) / denom;
}

// operator*
template <class T, class Allocator, class U>
vector<T, Allocator> operator*(vector<T, Allocator>&& vec, U mult)
{
    vector<T> result{std::move(vec)};
    std::transform(result.begin(), result.end(), result.begin(),
                   [=](const T& elem) { return elem * mult; });
    return result;
}

template <class T, class Allocator, class U>
vector<T, Allocator> operator*(const vector<T, Allocator>& vec, U mult)
{
    vector<T, Allocator> result{vec};
    return std::move(result) * mult;
}

template <class T, class U>
vector<typename std::remove_const<T>::type> operator*(util::array_view<T> vec,
                                                      U mult)
{
    vector<typename std::remove_const<T>::type> result{vec.begin(), vec.end()};
    return std::move(result) * mult;
}

template <class T, class Allocator, class U>
vector<T, Allocator> operator*(U mult, vector<T, Allocator>&& vec)
{
    vector<T> result{std::move(vec)};
    std::transform(result.begin(), result.end(), result.begin(),
                   [=](const T& elem) { return elem * mult; });
    return result;
}

template <class T, class Allocator, class U>
vector<T, Allocator> operator*(U mult, const vector<T, Allocator>& vec)
{
    vector<T, Allocator> result{vec};
    return std::move(result) * mult;
}

template <class T, class U>
vector<typename std::remove_const<T>::type> operator*(U mult,
                                                      util::array_view<T> vec)
{
    vector<typename std::remove_const<T>::type> result{vec.begin(), vec.end()};
    return std::move(result) * mult;
}

// norms
template <class T>
double l2norm(util::array_view<T> vec)
{
    return std::sqrt(std::accumulate(
        vec.begin(), vec.end(), 0.0,
        [](double accum, const T& elem) { return accum + elem * elem; }));
}

template <class T, class Allocator>
double l2norm(const vector<T, Allocator>& vec)
{
    return l2norm(util::array_view<const T>(vec));
}

template <class T>
double l1norm(util::array_view<T> vec)
{
    return std::accumulate(
        vec.begin(), vec.end(), 0.0,
        [](double accum, const T& elem) { return accum + std::abs(elem); });
}

template <class T, class Allocator>
double l1norm(const vector<T, Allocator>& vec)
{
    return l1norm(util::array_view<const T>(vec));
}
}
}
}
#endif
