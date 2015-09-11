/**
 * @file squared_hinge.cpp
 * @author Chase Geigle
 */

#include "classify/loss/squared_hinge.h"

namespace meta
{
namespace classify
{
namespace loss
{

const util::string_view squared_hinge::id = "squared-hinge";

double squared_hinge::loss(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z < 1)
        return 0.5 * (1 - z) * (1 - z);
    return 0;
}

double squared_hinge::derivative(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z < 1)
        return -expected * (1 - z);
    return 0;
}
}
}
}
