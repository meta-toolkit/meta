/**
 * @file radial_basis.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include "meta.h"

#ifndef _META_CLASSIFY_KERNEL_RADIAL_BASIS_H_
#define _META_CLASSIFY_KERNEL_RADIAL_BASIS_H_

namespace meta {
namespace classify {
namespace kernel {

/**
 * A radial basis function kernel for linear classifiers to adapt them to
 * data that is not linearly separable.
 *
 * Uses the form of:
 * \f$K(x, z) = \exp(\gamma||x-z||_2^2)\f$
 */
class radial_basis {
    public:
        radial_basis(double gamma) : gamma_{gamma} { }

        radial_basis(const radial_basis &) = default;
        radial_basis(radial_basis &&) = default;
        radial_basis & operator=(const radial_basis &) = default;
        radial_basis & operator=(radial_basis &&) = default;

        /**
         * Computes the value of \f$K(first, second)\f$.
         */
        template <class PostingsData>
        double operator()(const PostingsData & first,
                          const PostingsData & second) const;
    private:
        /**
         * \f$\gamma\f$, or equivalently \f$-\frac1{2\sigma^2}\f$, the
         * parameter for the radial basis function.
         */
        double gamma_;
};

}
}
}
#include "classify/kernel/radial_basis.tcc"
#endif
