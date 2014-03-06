/**
 * @file perceptron.cpp
 * @author Chase Geigle
 */

#include "classify/loss/perceptron.h"

namespace meta
{
namespace classify
{
namespace loss
{

const std::string perceptron::id = "perceptron";

double perceptron::loss(double prediction, int expected) const
{
    if (prediction * expected <= 0)
        return -expected * prediction;
    return 0;
}

double perceptron::derivative(double prediction, int expected) const
{
    if (prediction * expected <= 0)
        return -expected;
    return 0;
}
}
}
}
