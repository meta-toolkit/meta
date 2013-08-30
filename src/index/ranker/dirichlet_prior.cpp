/**
 * @file dirichlet_prior.cpp
 * @author Sean Massung
 */

#include "index/ranker/dirichlet_prior.h"

namespace meta {
namespace index {

dirichlet_prior::dirichlet_prior(double mu):
    _mu{mu}
{ /* nothing */ }

double dirichlet_prior::smoothed_prob(inverted_index & idx,
                              term_id t_id,
                              doc_id d_id) const
{
    double pc = 0.0; // TODO
    double numerator = idx.term_freq(t_id, d_id) + _mu * pc;
    double denominator = idx.doc_size(d_id) + _mu;
    return numerator / denominator;
}

double dirichlet_prior::doc_constant(inverted_index & idx, doc_id d_id) const
{
    return _mu / (idx.doc_size(d_id) + _mu);
}

}
}
