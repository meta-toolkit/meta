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

double absolute_discount::smoothed_prob(inverted_index & idx,
                              term_id t_id,
                              doc_id d_id) const
{
    double pc = static_cast<double>(idx.total_num_occurences(t_id))
        / idx.total_corpus_terms();
    double numerator = std::max<double>(idx.term_freq(t_id, d_id) - _delta, 0);
    double denominator = idx.doc_size(d_id);
    return numerator / denominator + doc_constant(idx, d_id) * pc;
}

double absolute_discount::doc_constant(inverted_index & idx, doc_id d_id) const
{
    double unique = 0.0; // TODO
    return _delta * unique / idx.doc_size(d_id);
}

}
}
