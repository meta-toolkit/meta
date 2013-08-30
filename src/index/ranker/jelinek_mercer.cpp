/**
 * @file jelinek_mercer.cpp
 * @author Sean Massung
 */

#include "index/ranker/jelinek_mercer.h"

namespace meta {
namespace index {

jelinek_mercer::jelinek_mercer(double lambda):
    _lambda{lambda}
{ /* nothing */ }

double jelinek_mercer::smoothed_prob(inverted_index & idx,
                              term_id t_id,
                              doc_id d_id) const
{
    double max_likelihood = static_cast<double>(idx.term_freq(t_id, d_id))
        / idx.doc_size(d_id);
    double collec_prob = 0.0;
    return (1.0 - _lambda) * max_likelihood + _lambda * collec_prob;
}

double jelinek_mercer::doc_constant(inverted_index & idx, doc_id d_id) const
{
    return _lambda;
}

}
}
