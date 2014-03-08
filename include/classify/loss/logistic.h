/**
 * @file logistic.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_LOGISTIC_LOSS_H_
#define _META_CLASSIFY_LOGISTIC_LOSS_H_

#include <cmath>
#include "classify/loss/loss_function.h"

namespace meta
{
namespace classify
{
namespace loss
{

struct logistic : public loss_function
{
    const static std::string id;
    double loss(double prediction, int expected) const override;
    double derivative(double prediction, int expected) const override;
};
}
}
}
#endif
