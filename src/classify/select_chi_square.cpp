/**
 * @file select_chi_square.cpp
 */

#include "classify/select_chi_square.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::string;
using std::pair;

using index::TermID;
using index::Document;

select_chi_square::select_chi_square(const vector<Document> & docs):
    select_simple(docs) { /* nothing */ }

double select_chi_square::calc_weight(TermID termID, const string & label) const
{
    double p_tc = term_and_class(termID, label);
    double p_ntnc = not_term_and_not_class(termID, label);
    double p_ntc = not_term_and_class(termID, label);
    double p_tnc = term_and_not_class(termID, label);
    double p_c = _pclass.at(label);
    double p_t = _pterm.at(termID);

    double numerator = p_tc * p_ntnc - p_ntc * p_tnc;
    numerator *= numerator;
    double denominator = p_c * (1.0 - p_c) * p_t * (1.0 - p_t);

    return numerator / denominator;
}

}
}
