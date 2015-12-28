/**
 * @file perceptron.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/perceptron.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
{
namespace loss
{

const util::string_view perceptron::id = "perceptron";

double perceptron::loss(double prediction, double expected) const
{
    if (prediction * expected <= 0)
        return -expected * prediction;
    return 0;
}

double perceptron::derivative(double prediction, double expected) const
{
    if (prediction * expected <= 0)
        return -expected;
    return 0;
}

void perceptron::save(std::ostream& out) const
{
    io::packed::write(out, id);
}
}
}
}
