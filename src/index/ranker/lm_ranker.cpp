/**
 * @file lm_ranker.cpp
 * @author Sean Massung
 */

#include <cmath>
#include "corpus/document.h"
#include "index/score_data.h"
#include "index/ranker/lm_ranker.h"

namespace meta
{
namespace index
{

const std::string language_model_ranker::id = "language-model";

double language_model_ranker::score_one(const score_data& sd)
{
    double ps = smoothed_prob(sd);
    double pc = static_cast<double>(sd.corpus_term_count) / sd.total_terms;

    return sd.query_term_count * std::log(ps / (doc_constant(sd) * pc));
}

double language_model_ranker::initial_score(const score_data& sd) const
{
    return sd.query.length() * std::log(doc_constant(sd));
}

}
}
