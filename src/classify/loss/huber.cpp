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

const util::string_view huber::id = "huber";

double huber::loss(double prediction, double expected) const
{
    double abs_diff = std::abs(prediction - expected);
    if (abs_diff <= 1)
        return abs_diff * abs_diff;
    return 2 * abs_diff - 1;
}

double huber::derivative(double prediction, double expected) const
{
    double diff = prediction - expected;
    if (std::abs(diff) <= 1)
        return 2 * diff;
    return (2 * diff) / (std::sqrt(diff * diff));
}
}
}
}
