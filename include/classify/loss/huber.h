/**
 * @file huber.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_HUBER_LOSS_H_
#define _META_CLASSIFY_HUBER_LOSS_H_

#include <cmath>

namespace meta {
namespace classify {
namespace loss {

struct huber {
    double loss(double prediction, int expected) const {
        double abs_diff = std::abs(prediction - expected);
        if (abs_diff <= 1)
            return abs_diff * abs_diff;
        return 2 * abs_diff - 1;
    }

    double derivative(double prediction, int expected) const {
        double diff = prediction - expected;
        if (std::abs(diff) <= 1)
            return 2 * diff;
        return (2 * diff) / (std::sqrt(diff * diff));
    }
};

}
}
}
#endif
