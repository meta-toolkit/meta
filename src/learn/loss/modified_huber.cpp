/**
 * @file modified_huber.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/modified_huber.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
{
namespace loss
{

const util::string_view modified_huber::id = "modified-huber";

double modified_huber::loss(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z < -1)
        return -2 * z;
    if (z >= 1)
        return 0;
    return 0.5 * (1 - z) * (1 - z);
}

double modified_huber::derivative(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z < -1)
        return -2 * expected;
    if (z >= 1)
        return 0;
    return -expected * (1 - z);
}

void modified_huber::save(std::ostream& out) const
{
    io::packed::write(out, id);
}
}
}
}
