/**
 * @file modified_huber.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_MODIFIED_HUBER_LOSS_H_
#define _META_CLASSIFY_MODIFIED_HUBER_LOSS_H_

namespace meta {
namespace classify {
namespace loss {

struct modified_huber {
    double loss(double prediction, int expected) const {
        double z = prediction * expected;
        if (z < -1)
            return -2 * z;
        if (z >= 1)
            return 0;
        return 0.5 * (1 - z) * (1 - z);
    }

    double derivative(double prediction, int expected) const {
        double z = prediction * expected;
        if (z < -1)
            return -2 * expected;
        if (z >= 1)
            return 0;
        return -expected * (1 - z);
    }
};

}
}
}
#endif
