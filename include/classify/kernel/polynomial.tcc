#include <cmath>

#include "classify/kernel/polynomial.h"
#include "util/mapping.h"

namespace meta {
namespace classify {
namespace kernel {

template <class PostingsData>
double polynomial::operator()(const PostingsData & first,
                              const PostingsData & second) const {
    double dot = c_;
    for(const auto & w : first->counts())
        dot += w.second * map::safe_at(second->counts(), w.first);
    return std::pow(dot, power_);
}

}
}
}
