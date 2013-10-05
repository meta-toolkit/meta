/**
 * @file squared_hinge.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_SQUARED_HINGE_LOSS_H_
#define _META_CLASSIFY_SQUARED_HINGE_LOSS_H_

namespace meta {
namespace classify {
namespace loss {

struct squared_hinge {
    double loss(double prediction, int expected) const {
        double z = prediction * expected;
        if (z < 1)
            return 0.5 * (1 - z) * (1 - z);
        return 0;
    }

    double derivative(double prediction, int expected) const {
        double z = prediction * expected;
        if (z < 1)
            return -expected * (1 - z);
        return 0;
    }
};

}
}
}
#endif
