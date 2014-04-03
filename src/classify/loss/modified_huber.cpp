/**
 * @file modified_huber.cpp
 * @author Chase Geigle
 */

#include "classify/loss/modified_huber.h"

namespace meta
{
namespace classify
{
namespace loss
{

const std::string modified_huber::id = "modified-huber";

double modified_huber::loss(double prediction, int expected) const
{
    double z = prediction * expected;
    if (z < -1)
        return -2 * z;
    if (z >= 1)
        return 0;
    return 0.5 * (1 - z) * (1 - z);
}

double modified_huber::derivative(double prediction, int expected) const
{
    double z = prediction * expected;
    if (z < -1)
        return -2 * expected;
    if (z >= 1)
        return 0;
    return -expected * (1 - z);
}
}
}
}
