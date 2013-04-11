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
using std::string;
using std::pair;

using index::TermID;
using index::Document;

select_corr_coeff::select_corr_coeff(const vector<Document> & docs):
    feature_select(docs) { /* nothing */ }

vector<pair<TermID, double>> select_corr_coeff::select()
{
    unordered_map<TermID, double> feature_weights;

    std::mutex _mutex;
    for(auto & c: _class_space)
    {
        parallel::parallel_for(_term_space.begin(), _term_space.end(), [&](const TermID t)
        {
            double cc = calc_corr_coeff(t, c);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if(feature_weights[t] < cc)
                    feature_weights[t] = cc;
            }
        });
    }

    return sort_terms(feature_weights);
}

double select_corr_coeff::calc_corr_coeff(TermID termID, const string & label)
{
    double p_tc = term_and_class(termID, label);
    double p_ntnc = not_term_and_not_class(termID, label);
    double p_ntc = not_term_and_class(termID, label);
    double p_tnc = term_and_not_class(termID, label);
    double p_c = _pclass[label];
    double p_t = _pterm[termID];

    double numerator = p_tc * p_ntnc - p_ntc * p_tnc;
    double denominator = sqrt(p_c * (1.0 - p_c) * p_t * (1.0 - p_t));

    return numerator / denominator;
}

}
}
