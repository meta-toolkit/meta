/**
 * @file hinge.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/hinge.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
{
namespace loss
{

const util::string_view hinge::id = "hinge";

double hinge::loss(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z < 1)
        return 1 - z;
    return 0;
}

double hinge::derivative(double prediction, double expected) const
{
    double z = prediction * expected;
    if (z < 1)
        return -expected;
    return 0;
}

void hinge::save(std::ostream& out) const
{
    io::packed::write(out, id);
}

}
}
}
