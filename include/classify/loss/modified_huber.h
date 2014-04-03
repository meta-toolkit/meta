/**
 * @file modified_huber.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_MODIFIED_HUBER_LOSS_H_
#define META_CLASSIFY_MODIFIED_HUBER_LOSS_H_

#include "classify/loss/loss_function.h"

namespace meta
{
namespace classify
{
namespace loss
{

/**
 * The modified huber loss function for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = \max(0, 1 - py)^2\f$ when \f$py \geq -1\f$
 * and \f$\phi(p, y) = -4py\f$ otherwise.
 */
struct modified_huber : public loss_function
{
    /**
     * The identifier for this loss function.
     */
    const static std::string id;

    double loss(double prediction, int expected) const override;
    double derivative(double prediction, int expected) const override;
};
}
}
}
#endif
