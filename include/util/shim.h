/**
 * @file shim.h
 * @author Chase Geigle
 * Contains functions from future C++ standards that we find useful before
 * standardization.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_UTIL_SHIM_H_
#define META_UTIL_SHIM_H_

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
