/**
 * @file jelinek_mercer.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "index/ranker/jelinek_mercer.h"
#include "index/score_data.h"

namespace meta
{
namespace index
{

const util::string_view jelinek_mercer::id = "jelinek-mercer";

jelinek_mercer::jelinek_mercer(float lambda) : lambda_{lambda}
{
    /* nothing */
}

float jelinek_mercer::smoothed_prob(const score_data& sd) const
{
    float max_likelihood = static_cast<float>(sd.doc_term_count)
                            / sd.doc_size;
    float pc = static_cast<float>(sd.corpus_term_count) / sd.total_terms;
    return (1.0f - lambda_) * max_likelihood + lambda_ * pc;
}

float jelinek_mercer::doc_constant(const score_data&) const
{
    return lambda_;
}

template <>
std::unique_ptr<ranker>
    make_ranker<jelinek_mercer>(const cpptoml::table& config)
{
    if (auto lambda = config.get_as<double>("lambda"))
        return make_unique<jelinek_mercer>(*lambda);
    return make_unique<jelinek_mercer>();
}
}
}
