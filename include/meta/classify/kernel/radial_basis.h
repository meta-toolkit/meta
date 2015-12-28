/**
 * @file radial_basis.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include "meta/classify/kernel/kernel.h"
#include "meta/classify/kernel/kernel_factory.h"

#ifndef META_CLASSIFY_KERNEL_RADIAL_BASIS_H_
#define META_CLASSIFY_KERNEL_RADIAL_BASIS_H_

namespace meta
{
namespace classify
{
namespace kernel
{

/**
 * A radial basis function kernel for linear classifiers to adapt them to
 * data that is not linearly separable.
 *
 * Uses the form of:
 * \f$K(x, z) = \exp(\gamma||x-z||_2^2)\f$
 */
class radial_basis : public kernel
{
  public:
    /**
     * Constructs a new radial_basis kernel with the given parameter.
     *
     * @param gamma The parameter for the radial basis function.
     */
    radial_basis(double gamma);

    /**
     * Loads an rbf kernel from a stream.
     * @param in The stream to read from
     */
    radial_basis(std::istream& in);

    double operator()(const feature_vector& first,
                      const feature_vector& second) const override;

    void save(std::ostream& out) const override;

    static const util::string_view id;

  private:
    /**
     * \f$\gamma\f$, or equivalently \f$-\frac1{2\sigma^2}\f$, the
     * parameter for the radial basis function.
     */
    double gamma_;
};

/**
 * Specialization of the factory method used to create rbf kernels.
 */
template <>
std::unique_ptr<kernel> make_kernel<radial_basis>(const cpptoml::table&);
}
}
}
#endif
