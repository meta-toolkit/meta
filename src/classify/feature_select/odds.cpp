/**
 * @file odds.cpp
 */

#include "classify/feature_select/odds.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::pair;
using index::document;

odds_ratio::odds_ratio(const vector<document> & docs):
    select_simple(docs) { /* nothing */ }

double odds_ratio::calc_weight(term_id termID, const class_label & label) const
{
    double p_tc = term_and_class(termID, label);
    double p_tnc = term_and_not_class(termID, label);
    double denominator = (1.0 - p_tc) * p_tnc;

    if(denominator == 0.0)
        return 0.0;

    double numerator = p_tc * (1.0 - p_tnc);

    return log(1 + (numerator / denominator));
}

}
}
