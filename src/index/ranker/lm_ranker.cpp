/**
 * @file lm_ranker.cpp
 * @author Sean Massung
 */

#include "index/ranker/lm_ranker.h"

namespace meta {
namespace index {

double language_model_ranker::score_one(const score_data & sd) const
{
    double ps = smoothed_prob(sd);
    double doc_const = doc_constant(sd);
    double pc = static_cast<double>(sd.idx.total_num_occurences(sd.t_id))
        / sd.total_terms;

    return ps / doc_const * pc + sd.query.length() * log(1 + doc_const);
}

}
}
