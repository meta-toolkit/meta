/**
 * @file clonable.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_CLONABLE_H_
#define META_UTIL_CLONABLE_H_

#include <memory>

#include "meta/config.h"
#include "meta/util/shim.h"

namespace meta
{
namespace util
{

/**
 * Template class to facilitate polymorphic cloning. Use in place of an
 * ordinary base class, with first parameter being the root of the
 * inheritance hierarchy, the second being the desired base class, and the
 * third being the current class (CRTP style).
 */
template <class Root, class Base, class Derived>
class multilevel_clonable : public Base
{
  public:
    /// Inherit the constructors from the base class
    using Base::Base;

    /**
     * Clones the given object. This requires that the Derived class be
     * capable of being copy-constructed.
     *
     * @return a unique_ptr to the Root type object.
     */
    virtual std::unique_ptr<Root> clone() const
    {
        return make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

/**
 * Template alias to facilitate polymorphic cloning. Use in place of an
 * ordinary base class, with first parameter being the base class and the
 * second parameter being the current class (CRTP style).
 */
template <class Base, class Derived>
using clonable = multilevel_clonable<Base, Base, Derived>;
}
}
#endif
