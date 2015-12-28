/**
 * @file sigmoid.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include "meta/classify/kernel/kernel.h"
#include "meta/classify/kernel/kernel_factory.h"

#ifndef META_CLASSIFY_KERNEL_SIGMOID_H_
#define META_CLASSIFY_KERNEL_SIGMOID_H_

namespace meta
{
namespace classify
{
namespace kernel
{

/**
 * A sigmoid kernel function for a linear classifier to adapt it to data
 * that is not linearly separable.
 *
 * Uses the general form of:
 * \f$K(x,y) = \tanh(\alpha x^T y + c)\f$
 */
class sigmoid : public kernel
{
  public:
    /**
     * Constructs a new sigmoid kernel with the given parameters.
     *
     * @param alpha \f$\alpha\f$, the coefficient for the dot product
     * @param c \f$c\f$, the additional scalar term
     */
    sigmoid(double alpha, double c);

    /**
     * Loads a sigmoid kernel from a stream.
     * @param in The stream to read from
     */
    sigmoid(std::istream& in);

    double operator()(const feature_vector& first,
                      const feature_vector& second) const override;

    void save(std::ostream& out) const override;

    static const util::string_view id;

  private:
    /**
     * \f$\alpha\f$, the coefficient for the dot product.
     */
    double alpha_;

    /**
     * \f$c\f$, the additional scalar term
     */
    double c_;
};

/**
 * Specialization of the factory method used to create sigmoid kernels.
 */
template <>
std::unique_ptr<kernel> make_kernel<sigmoid>(const cpptoml::table&);
}
}
}
#endif
