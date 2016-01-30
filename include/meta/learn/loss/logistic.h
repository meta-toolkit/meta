/**
 * @file logistic.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_LOGISTIC_LOSS_H_
#define META_CLASSIFY_LOGISTIC_LOSS_H_

#include <cmath>
#include "meta/learn/loss/loss_function.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace learn
{
namespace loss
{

/**
 * The logistic loss for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = \ln(1 + e^{-py})\f$, this loss is suitable
 * for binary classification.
 */
struct logistic : public loss_function
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
