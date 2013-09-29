#include <cmath>
#include <unordered_set>

#include "classify/kernel/radial_basis.h"
#include "util/common.h"

namespace meta {
namespace classify {
namespace kernel {

template <class PostingsData>
double radial_basis::operator()(const PostingsData & first,
                                const PostingsData & second) const {
    auto first_c = first->counts();
    auto second_c = second->counts();
    double dist = 0;
    std::unordered_set<term_id> keyspace;
    for(const auto & p : first_c)
        keyspace.insert(p.first);
    for(const auto & p : second_c)
        keyspace.insert(p.first);
    for(const auto & t_id : keyspace) {
        auto delta = common::safe_at(first_c, t_id)
                     - common::safe_at(second_c, t_id);
        dist += delta * delta;
    }
    return std::exp(gamma_ * dist);
}

}
}
}
