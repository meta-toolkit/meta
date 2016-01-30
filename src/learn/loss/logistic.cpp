/**
 * @file logistic.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/logistic.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
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
