/**
 * @file polynomial.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include "meta.h"

#ifndef META_CLASSIFY_KERNEL_POLYNOMIAL_H_
#define META_CLASSIFY_KERNEL_POLYNOMIAL_H_

namespace meta
{
namespace classify
{
namespace kernel
{

/**
 * A polynomial kernel function for a linear classifier to adapt it to
 * data that is not linearly separable.
 *
 * Uses the general form of:
 * \f$K(x,z) = (x^T z + c)^p\f$
 */
class polynomial
{
  public:
    /**
     * Constructs a new polynomial kernel with the given parameters.
     *
     * @param power \f$p\f$, the power for the polynomial kernel.
     * @param c \f$c\f$, the additional scalar term to allow for the
     *  use of existing features in the original space
     */
    polynomial(uint8_t power = 1, double c = 1) : power_{power}, c_{c}
    {
        // nothing
    }

    /**
     * Computes the value of \f$K(first, second)\f$.
     */
    template <class PostingsData>
    double operator()(const PostingsData& first,
                      const PostingsData& second) const;

  private:
    /**
     * \f$p\f$, the power for the kernel
     */
    uint8_t power_;

    /**
     * \f$c\f$, the scalar term for the kernel
     */
    double c_;
};
}
}
}

#include "classify/kernel/polynomial.tcc"
#endif
