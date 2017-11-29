/**
 * @file dirichlet_prior.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "meta/index/ranker/dirichlet_prior.h"
#include "meta/index/score_data.h"

namespace meta
{
namespace index
{

const util::string_view dirichlet_prior::id = "dirichlet-prior";
const constexpr float dirichlet_prior::default_mu;

dirichlet_prior::dirichlet_prior(float mu) : mu_{mu}
{
    // nothing
}

dirichlet_prior::dirichlet_prior(std::istream& in)
    : mu_{io::packed::read<float>(in)}
{
    // nothing
}

void dirichlet_prior::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, mu_);
}

float dirichlet_prior::smoothed_prob(const score_data& sd) const
{
    float pc = static_cast<float>(sd.corpus_term_count) / sd.total_terms;
    float numerator = sd.doc_term_count + mu_ * pc;
    float denominator = sd.doc_size + mu_;
    return numerator / denominator;
}

float dirichlet_prior::doc_constant(const score_data& sd) const
{
    return mu_ / (sd.doc_size + mu_);
}

template <>
std::unique_ptr<ranker>
    make_ranker<dirichlet_prior>(const cpptoml::table& config)
{
    auto mu = config.get_as<double>("mu").value_or(dirichlet_prior::default_mu);
    if (mu < 0)
        throw ranker_exception{"dirichlet-prior mu must be >= 0"};
    return make_unique<dirichlet_prior>(mu);
}
}
}
