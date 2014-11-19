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

namespace meta
{

#ifndef META_HAS_STD_MAKE_UNIQUE
/**
 * Constructs a unique ptr in place.
 * @param args The parameters to the constructor
 * @return a unique_ptr<T>
 */
template <class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}
#else
using std::make_unique;
#endif
}
#endif
