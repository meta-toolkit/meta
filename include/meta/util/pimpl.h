/**
 * @file pimpl.h
 * @author Chase Geigle
 * @see http://herbsutter.com/gotw/_101/
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_PIMPL_H_
#define META_UTIL_PIMPL_H_

#include <memory>

#include "meta/config.h"
#include "meta/util/shim.h"

namespace meta
{
namespace util
{

/**
 * Class to assist in simple pointer-to-implementation classes.
 */
template <class Impl>
class pimpl
{
  public:
    /**
     * Constructor.
     */
    pimpl();

    /**
     * Move constructor.
     */
    pimpl(pimpl&&);

    /**
     * Move assignment.
     */
    pimpl& operator=(pimpl&&);

    /**
     * Forwarding constructor.
     * @param args The arguments to forward to the Impl class.
     */
    template <class... Args>
    pimpl(Args&&... args);

    /**
     * Destructor.
     */
    ~pimpl();

    /**
     * Member access operator.
     * @return a pointer to the underlying Impl class (member access).
     */
    Impl* operator->();

    /**
     * Member access operator. Const version.
     * @return a pointer to the underlying Impl class (member access).
     */
    const Impl* operator->() const;

    /**
     * Dereference operator.
     * @return the underlying Impl class.
     */
    Impl& operator*();

    /**
     * Dereference operator. Const version.
     * @return the underlying Impl class.
     */
    const Impl& operator*() const;

  private:
    /// The internal pointer to the Impl class.
    std::unique_ptr<Impl> impl_;
};
}
}

#endif
