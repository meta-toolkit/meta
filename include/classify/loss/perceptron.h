/**
 * @file perceptron.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_PERCEPTRON_LOSS_H_
#define _META_CLASSIFY_PERCEPTRON_LOSS_H_

namespace meta {
namespace classify {
namespace loss {

struct perceptron {
    double loss(double prediction, int expected) const {
        if( prediction * expected <= 0 )
            return -expected * prediction;
        return 0;
    }

    double derivative(double prediction, int expected) const {
        if( prediction * expected <= 0 )
            return -expected;
        return 0;
    }
};

}
}
}
#endif
