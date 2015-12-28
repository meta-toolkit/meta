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
#include "meta/learn/loss/loss_function.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace learn
{
namespace loss
{

/**
 * The huber loss for SGD algorithms.
 *
 * Defined as \f$\phi(p, y) = (p - y)^2\f$ when \f$|p-y| \leq 1\f$ and
 * \f$\phi(p, y) = 2|p - y| - 1\f$ otherwise.
 *
 * This loss is suitable for regression problems.
 */
struct huber : public loss_function
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
