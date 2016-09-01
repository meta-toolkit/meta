/**
 * @file shim.h
 * @author Chase Geigle
 * Contains functions from future C++ standards that we find useful before
 * standardization.
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_SHIM_H_
#define META_UTIL_SHIM_H_

#include <memory>

#include "meta/config.h"

namespace meta
{

#if META_HAS_STD_MAKE_UNIQUE == 0

namespace detail
{
template <class T>
struct unique_if
{
    using single_object = std::unique_ptr<T>;
};

template <class T>
struct unique_if<T[]>
{
    using unknown_bound = std::unique_ptr<T[]>;
};
}

/**
 * Constructs a unique ptr in place.
 * @param args The parameters to the constructor
 * @return a unique_ptr<T>
 */
template <class T, class... Args>
typename detail::unique_if<T>::single_object make_unique(Args&&... args)
{
    return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template <class T>
typename detail::unique_if<T>::unknown_bound make_unique(std::size_t size)
{
    return std::unique_ptr<T>{new typename std::remove_extent<T>::type[size]()};
}
#else
using std::make_unique;
#endif
}
#endif
