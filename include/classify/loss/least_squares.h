/**
 * @file least_squares.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_LEAST_SQUARES_LOSS_H_
#define _META_CLASSIFY_LEAST_SQUARES_LOSS_H_

namespace meta {
namespace classify {
namespace loss {

struct least_squares {
    double loss(double prediction, int expected) const {
        return 0.5 * (prediction - expected) * (prediction - expected);
    }

    double derivative(double prediction, int expected) const {
        return prediction - expected;
    }
};

}
}
}
#endif
