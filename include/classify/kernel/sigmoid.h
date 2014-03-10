/**
 * @file sigmoid.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include <cmath>

#include "classify/kernel/polynomial.h"
#include "meta.h"

#ifndef META_CLASSIFY_KERNEL_SIGMOID_H_
#define META_CLASSIFY_KERNEL_SIGMOID_H_

namespace meta {
namespace classify {
namespace kernel {

/**
 * A sigmoid kernel function for a linear classifier to adapt it to data
 * that is not linearly separable.
 *
 * Uses the general form of:
 * \f$K(x,y) = \tanh(\alphax^\intercal y + c)\f$
 */
class sigmoid {
    public:
        /**
         * Constructs a new sigmoid kernel with the given parameters.
         *
         * @param alpha \f$\alpha\f$, the coefficient for the dot product
         * @param c \f$c\f$, the additional scalar term
         */
        sigmoid(double alpha, double c):
            alpha_{alpha}, c_{c} { }

        sigmoid(const sigmoid &) = default;
        sigmoid(sigmoid &&) = default;
        sigmoid & operator=(const sigmoid &) = default;
        sigmoid & operator=(sigmoid &&) = default;

        /**
         * Computes the value of \f$K(first, second)\f$.
         */
        template <class PostingsData>
        double operator()(const PostingsData & first,
                          const PostingsData & second) const;

    private:
        /**
         * \f$\alpha\f$, the coefficient for the dot product.
         */
        double alpha_;

        /**
         * \f$c\f$, the additional scalar term
         */
        double c_;

        polynomial dot_{1, 0.0};
};


}
}
}

#include "classify/kernel/sigmoid.tcc"
#endif
