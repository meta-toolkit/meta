/**
 * @file shim.h
 * Contains functions from future C++ standards that we find useful before
 * standardization.
 */

#ifndef _META_UTIL_SHIM_H_
#define _META_UTIL_SHIM_H_

#include <memory>

namespace meta
{

/**
 * Constructs a unique ptr in place.
 * @param args The parameters to the constructor
 */
template <class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}
}
#endif
