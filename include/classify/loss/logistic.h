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

namespace meta {
namespace classify {
namespace loss {

struct logistic {
    double loss(double prediction, int expected) const {
        return std::log(1+std::exp(-prediction * expected));
    }

    double derivative(double prediction, int expected) const {
        return -expected / (std::exp(prediction * expected) + 1);
    }
};

}
}
}
#endif
