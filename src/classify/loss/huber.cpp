/**
 * @file huber.cpp
 * @author Chase Geigle
 */

#include "classify/loss/huber.h"

namespace meta
{
namespace classify
{
namespace loss
{

const std::string huber::id = "huber";

double huber::loss(double prediction, int expected) const
{
    double abs_diff = std::abs(prediction - expected);
    if (abs_diff <= 1)
        return abs_diff * abs_diff;
    return 2 * abs_diff - 1;
}

double huber::derivative(double prediction, int expected) const
{
    double diff = prediction - expected;
    if (std::abs(diff) <= 1)
        return 2 * diff;
    return (2 * diff) / (std::sqrt(diff * diff));
}
}
}
}
