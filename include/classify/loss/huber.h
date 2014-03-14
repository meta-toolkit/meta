/**
 * @file huber.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_HUBER_LOSS_H_
#define META_CLASSIFY_HUBER_LOSS_H_

#include <cmath>
#include "classify/loss/loss_function.h"

namespace meta
{
namespace classify
{
namespace loss
{

/**
 * The huber loss for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = (p - y)^2\f$ when \f$|p-y| \leq 1\f$ and
 * \f$\phi(p, y) = 2|p - y| - 1\f$ otherwise.
 */
struct huber : public loss_function
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
