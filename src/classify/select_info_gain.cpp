/**
 * @file select_info_gain.cpp
 */

#include "classify/select_info_gain.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::pair;
using index::Document;

select_info_gain::select_info_gain(const vector<Document> & docs):
    select_simple(docs) { /* nothing */ }

double select_info_gain::calc_weight(term_id termID, const class_label & label) const
{
    double p_tc = term_and_class(termID, label);
    double p_ntnc = not_term_and_not_class(termID, label);
    double p_ntc = not_term_and_class(termID, label);
    double p_tnc = term_and_not_class(termID, label);
    double p_c = _pclass.at(label);
    double p_t = _pterm.at(termID);
    double p_nc = 1.0 - p_c;
    double p_nt = 1.0 - p_t;

    double gain_tc = p_tc * log(1 + (p_tc / (p_t * p_c)));
    double gain_ntnc = p_ntnc * log(1 + (p_ntnc / (p_nt * p_nc)));
    double gain_ntc = p_ntc * log(1 + (p_ntc / (p_nt * p_c)));
    double gain_tnc = p_tnc * log(1 + (p_tnc / (p_t * p_nc)));

    return gain_tc + gain_ntnc + gain_ntc + gain_tnc;
}

}
}
