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

namespace meta {
namespace classify {
namespace loss {

struct hinge : public loss_function {
    double loss(double prediction, int expected) const override;
    double derivative(double prediction, int expected) const override;
    const static std::string id;
};

}
}
}
#endif
