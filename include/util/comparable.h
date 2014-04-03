/**
 * @file comparable.h
 * @author Chase Geigle
 * Defines a CRTP base class that allows for inheritance of comparator
 * operations given that the base class defines operator<().
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_COMPARABLE_H_
#define META_COMPARABLE_H_

namespace meta
{
namespace util
{

/**
 * A CRTP base class that allows for inheritance of all comparator
 * operators given that the derived class defines an operator<(). All
 * operations are defined in terms of Derived's operator<().
 */
template <class Derived>
class comparable
{
  public:
    /**
     * Determines if two comparables are equivalent, as defined by their
     * operator<.
     *
     * @param lhs
     * @param rhs
     * @return whether lhs == rhs
     */
    friend bool operator==(const comparable& lhs, const comparable& rhs)
    {
        return !(lhs.as_derived() < rhs.as_derived())
               && !(rhs.as_derived() < lhs.as_derived());
    }

    /**
     * Determines if two comparables are *not* equivalent, as defined by
     * negation of the comparable::operator==.
     *
     * @param lhs
     * @param rhs
     * @return whether lhs != rhs
     */
    friend bool operator!=(const comparable& lhs, const comparable& rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * @param lhs
     * @param rhs
     * @return whether lhs > rhs, as defined by their operator<.
     */
    friend bool operator>(const comparable& lhs, const comparable& rhs)
    {
        return rhs.as_derived() < lhs.as_derived();
    }

    /**
     * @param lhs
     * @param rhs
     * @return whether lhs <= rhs, as defined by their operator< and
     * comparable::operator==.
     */
    friend bool operator<=(const comparable& lhs, const comparable& rhs)
    {
        return lhs.as_derived() < rhs.as_derived() || lhs == rhs;
    }

    /**
     * @param lhs
     * @param rhs
     * @return whether lhs >= rhs, as defined by comparable::operator> and
     * comparable::operator==.
     */
    friend bool operator>=(const comparable& lhs, const comparable& rhs)
    {
        return lhs > rhs || lhs == rhs;
    }

  private:
    /**
     * Helper method to cast the current comparable to its Derived
     * representation.
     *
     * @return the Derived form of the current comparable
     */
    inline const Derived& as_derived() const
    {
        return static_cast<const Derived&>(*this);
    }
};
}
}
#endif
