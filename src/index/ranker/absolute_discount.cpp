/**
 * @file absolute_discount.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include "index/ranker/absolute_discount.h"

namespace meta {
namespace index {

absolute_discount::absolute_discount(double delta):
    _delta{delta}
{ /* nothing */ }

double absolute_discount::smoothed_prob(const score_data & sd) const
{
    double pc = static_cast<double>(sd.idx.total_num_occurences(sd.t_id))
        / sd.total_terms;
    double numerator = std::max<double>(sd.doc_term_count - _delta, 0);
    double denominator = sd.doc_size;
    return numerator / denominator + doc_constant(sd) * pc;
}

double absolute_discount::doc_constant(const score_data & sd) const
{
    double unique = 0.0; // TODO
    return _delta * unique / sd.doc_size;
}

}
}
