/**
 * @file radial_basis.tcc
 * @author Chase Geigle
 */

#include <cmath>
#include <unordered_set>

#include "classify/kernel/radial_basis.h"

namespace meta
{
namespace classify
{
namespace kernel
{

template <class PostingsData>
double radial_basis::operator()(const PostingsData& first,
                                const PostingsData& second) const
{
    double dist = 0;
    std::unordered_set<term_id> keyspace;
    for (const auto& p : first->counts())
        keyspace.insert(p.first);
    for (const auto& p : second->counts())
        keyspace.insert(p.first);
    for (const auto& t_id : keyspace)
    {
        auto delta = first->count(t_id) - second->count(t_id);
        dist += delta * delta;
    }
    return std::exp(gamma_ * dist);
}
}
}
}
