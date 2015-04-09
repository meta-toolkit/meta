/**
 * @file logistic.cpp
 * @author Chase Geigle
 */

#include "classify/loss/logistic.h"

namespace meta
{
namespace classify
{
namespace loss
{

const std::string logistic::id = "logistic";

double logistic::loss(double prediction, double expected) const
{
    return std::log(1 + std::exp(-prediction * expected));
}

double logistic::derivative(double prediction, double expected) const
{
    return -expected / (std::exp(prediction * expected) + 1);
}
}
}
}
