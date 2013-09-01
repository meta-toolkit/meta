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

double jelinek_mercer::smoothed_prob(const score_data & sd) const
{
    double max_likelihood = static_cast<double>(sd.doc_term_count)
        / sd.doc_size;
    double pc = static_cast<double>(sd.corpus_term_count) / sd.total_terms;
    return (1.0 - _lambda) * max_likelihood + _lambda * pc;
}

double jelinek_mercer::doc_constant(const score_data & sd) const
{
    return _lambda;
}

}
}
