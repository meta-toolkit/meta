/**
 * @file hinge.cpp
 * @author Chase Geigle
 */

#include "classify/loss/hinge.h"

namespace meta
{
namespace classify
{
namespace loss
{

const std::string hinge::id = "hinge";

double hinge::loss(double prediction, int expected) const
{
    double z = prediction * expected;
    if (z < 1)
        return 1 - z;
    return 0;
}

double hinge::derivative(double prediction, int expected) const
{
    double z = prediction * expected;
    if (z < 1)
        return -expected;
    return 0;
}

}
}
}
