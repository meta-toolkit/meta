/**
 * @file clonable.h
 * @author Chase Geigle
 */

#ifndef _META_UTIL_CLONABLE_H_
#define _META_UTIL_CLONABLE_H_

#include <memory>
#include "util/shim.h"

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
      using Base::Base;

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
