/**
 * @file modified_huber.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_MODIFIED_HUBER_LOSS_H_
#define META_CLASSIFY_MODIFIED_HUBER_LOSS_H_

#include "meta/learn/loss/loss_function.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace learn
{
namespace loss
{

/**
 * The modified huber loss function for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = \max(0, 1 - py)^2\f$ when \f$py \geq -1\f$
 * and \f$\phi(p, y) = -4py\f$ otherwise.
 *
 * This loss is suitable for binary classification problems.
 */
struct modified_huber : public loss_function
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
