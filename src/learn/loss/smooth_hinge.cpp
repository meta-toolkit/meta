/**
 * @file smooth_hinge.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/smooth_hinge.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
{
namespace loss
{

const util::string_view smooth_hinge::id = "smooth-hinge";

double smooth_hinge::loss(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z <= 0)
        return 0.5 - z;
    if (z >= 1)
        return 0;
    return 0.5 * (1 - prediction * expected) * (1 - prediction * expected);
}

double smooth_hinge::derivative(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z <= 0)
        return -expected;
    if (z >= 1)
        return 0;
    return -expected * (1 - z);
}

void smooth_hinge::save(std::ostream& out) const
{
    io::packed::write(out, id);
}
}
}
}
