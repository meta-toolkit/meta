/**
 * @file lm_ranker.cpp
 * @author Sean Massung
 */

#include <cmath>
#include "meta/corpus/document.h"
#include "meta/math/fastapprox.h"
#include "meta/index/score_data.h"
#include "meta/index/ranker/lm_ranker.h"

namespace meta
{
namespace index
{

const util::string_view language_model_ranker::id = "language-model";

float language_model_ranker::score_one(const score_data& sd)
{
    float ps = smoothed_prob(sd);
    float pc = static_cast<float>(sd.corpus_term_count) / sd.total_terms;
    return sd.query_term_weight
           * fastapprox::fastlog(ps / (doc_constant(sd) * pc));
}

float language_model_ranker::initial_score(const score_data& sd) const
{
    return sd.query_length * fastapprox::fastlog(doc_constant(sd));
}
}
}
