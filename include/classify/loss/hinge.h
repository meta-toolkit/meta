/**
 * @file hinge.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_HINGE_LOSS_H_
#define META_CLASSIFY_HINGE_LOSS_H_

#include <algorithm>
#include "classify/loss/loss_function.h"

namespace meta
{
namespace classify
{
namespace loss
{

/**
 * The hinge loss for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = \max(0,1-py)\f$, with
 * \f$\phi^\prime(p,y) = -y\f$ if \f$py < 1\f$, 0 otherwise. (Technically,
 * the derivative doesn't always exist for straight hinge-loss, so this is
 * a subgradient approach. You can avoid this problem by using a smoothed
 * version of the hinge loss, like smooth_hinge).
 */
struct hinge : public loss_function
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
