/**
 * @file dirichlet_prior.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "index/ranker/dirichlet_prior.h"
#include "index/score_data.h"

namespace meta
{
namespace index
{

const std::string dirichlet_prior::id = "dirichlet-prior";

dirichlet_prior::dirichlet_prior(double mu) : mu_{mu}
{
    /* nothing */
}

double dirichlet_prior::smoothed_prob(const score_data& sd) const
{
    double pc = static_cast<double>(sd.corpus_term_count) / sd.total_terms;
    double numerator = sd.doc_term_count + mu_ * pc;
    double denominator = sd.doc_size + mu_;
    return numerator / denominator;
}

double dirichlet_prior::doc_constant(const score_data& sd) const
{
    return mu_ / (sd.doc_size + mu_);
}

template <>
std::unique_ptr<ranker>
    make_ranker<dirichlet_prior>(const cpptoml::table& config)
{
    if (auto mu = config.get_as<double>("mu"))
        return make_unique<dirichlet_prior>(*mu);
    return make_unique<dirichlet_prior>();
}
}
}
