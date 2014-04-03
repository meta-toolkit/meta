/**
 * @file sigmoid.tcc
 * @author Chase Geigle
 */

#include "classify/kernel/sigmoid.h"

namespace meta
{
namespace classify
{
namespace kernel
{

template <class PostingsData>
double sigmoid::operator()(const PostingsData& first,
                           const PostingsData& second) const
{
    return std::tanh(alpha_ * dot_(first, second) + c_);
}
}
}
}
