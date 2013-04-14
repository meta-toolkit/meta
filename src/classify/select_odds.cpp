/**
 * @file select_odds.cpp
 */

#include "classify/select_odds.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::pair;
using index::Document;

select_odds_ratio::select_odds_ratio(const vector<Document> & docs):
    select_simple(docs) { /* nothing */ }

double select_odds_ratio::calc_weight(term_id termID, const class_label & label) const
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
