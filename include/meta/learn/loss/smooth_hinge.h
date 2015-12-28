/**
 * @file smooth_hinge.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_SMOOTH_HINGE_LOSS_H_
#define META_CLASSIFY_SMOOTH_HINGE_LOSS_H_

#include "meta/learn/loss/loss_function.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace learn
{
namespace loss
{

/**
 * The smooth hinge loss function for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = \frac12 \max(0, 1 - py)^2\f$
 * if \f$py \geq 0\f$ and \f$\phi(p, y) = \frac12 - py\f$ otherwise.
 *
 * This loss is suitable for binary classification.
 */
struct smooth_hinge : public loss_function
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
