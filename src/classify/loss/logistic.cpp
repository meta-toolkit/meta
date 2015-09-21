/**
 * @file logistic.cpp
 * @author Chase Geigle
 */

#include "classify/loss/logistic.h"
#include "io/packed.h"

namespace meta
{
namespace classify
{
namespace loss
{

const util::string_view logistic::id = "logistic";

double logistic::loss(double prediction, double expected) const
{
    return std::log(1 + std::exp(-prediction * expected));
}

double logistic::derivative(double prediction, double expected) const
{
    return -expected / (std::exp(prediction * expected) + 1);
}

void logistic::save(std::ostream& out) const
{
    io::packed::write(out, id);
}
}
}
}
