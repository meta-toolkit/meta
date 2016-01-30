/**
 * @file polynomial.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */


#include "meta/classify/kernel/kernel.h"
#include "meta/classify/kernel/kernel_factory.h"

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
class polynomial : public kernel
{
  public:
    const static constexpr uint8_t default_power = 1;
    const static constexpr double default_c = 1;
    const static util::string_view id;

    /**
     * Constructs a new polynomial kernel with the given parameters.
     *
     * @param power \f$p\f$, the power for the polynomial kernel.
     * @param c \f$c\f$, the additional scalar term to allow for the
     *  use of existing features in the original space
     */
    polynomial(uint8_t power = default_power, double c = default_c);

    /**
     * Loads a polynomial kernel from a stream.
     */
    polynomial(std::istream& in);

    /**
     * Computes the value of \f$K(first, second)\f$.
     */
    double operator()(const feature_vector& first,
                      const feature_vector& second) const override;

    void save(std::ostream& out) const override;

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

/**
 * Specialization of the factory method used to create polynomial kernels.
 */
template <>
std::unique_ptr<kernel> make_kernel<polynomial>(const cpptoml::table&);
}
}
}
#endif
