/**
 * @file select_corr.cpp
 */

#include "classify/select_corr.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::pair;
using index::document;

select_corr_coeff::select_corr_coeff(const vector<document> & docs):
    select_simple(docs) { /* nothing */ }

double select_corr_coeff::calc_weight(term_id termID, const class_label & label) const
{
    double p_tc = term_and_class(termID, label);
    double p_ntnc = not_term_and_not_class(termID, label);
    double p_ntc = not_term_and_class(termID, label);
    double p_tnc = term_and_not_class(termID, label);
    double p_c = _pclass.at(label);
    double p_t = _pterm.at(termID);

    double numerator = p_tc * p_ntnc - p_ntc * p_tnc;
    double denominator = sqrt(p_c * (1.0 - p_c) * p_t * (1.0 - p_t));

    return numerator / denominator;
}

}
}
