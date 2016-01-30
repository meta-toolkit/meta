/**
 * @file least_squares.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/least_squares.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
{
namespace loss
{

const util::string_view least_squares::id = "least-squares";

double least_squares::loss(double prediction, double expected) const
{
    return 0.5 * (prediction - expected) * (prediction - expected);
}

double least_squares::derivative(double prediction, double expected) const
{
    return prediction - expected;
}

void least_squares::save(std::ostream& out) const
{
    io::packed::write(out, id);
}
}
}
}
