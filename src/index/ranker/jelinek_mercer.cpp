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

const std::string jelinek_mercer::id = "jelinek-mercer";

jelinek_mercer::jelinek_mercer(double lambda) : lambda_{lambda}
{
    /* nothing */
}

double jelinek_mercer::smoothed_prob(const score_data& sd) const
{
    double max_likelihood = static_cast<double>(sd.doc_term_count)
                            / sd.doc_size;
    double pc = static_cast<double>(sd.corpus_term_count) / sd.total_terms;
    return (1.0 - lambda_) * max_likelihood + lambda_ * pc;
}

double jelinek_mercer::doc_constant(const score_data&) const
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
