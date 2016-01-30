/**
 * @file huber.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/huber.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
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

void huber::save(std::ostream& out) const
{
    io::packed::write(out, id);
}
}
}
}
