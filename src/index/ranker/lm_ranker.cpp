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
    double doc_const = doc_constant(sd);
    double pc = static_cast<double>(sd.corpus_term_count) / sd.total_terms;

    return ps / doc_const * pc + sd.query.length() * std::log(1 + doc_const);
}
}
}
