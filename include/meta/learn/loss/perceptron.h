/**
 * @file perceptron.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_PERCEPTRON_LOSS_H_
#define META_CLASSIFY_PERCEPTRON_LOSS_H_

#include "meta/learn/loss/loss_function.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace learn
{
namespace loss
{

/**
 * The perceptron loss function for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = -py\f$ if \f$py \leq 0\f$, 0 otherwise.
 */
struct perceptron : public loss_function
{
    /**
     * The identifier for this loss function.
     */
    const static util::string_view id;

    double loss(double prediction, double expected) const override;
    double derivative(double prediction, double expected) const override;
    void save(std::ostream& out) const override;
};
}
}
}
#endif
