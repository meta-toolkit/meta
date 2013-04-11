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
using std::string;

using index::Document;
using index::TermID;

select_info_gain::select_info_gain(const vector<Document> & docs):
    feature_select(docs) { /* nothing */ }

vector<pair<TermID, double>> select_info_gain::select()
{
    unordered_map<TermID, double> feature_weights;

    std::mutex _mutex;
    for(auto & c: _class_space)
    {
        parallel::parallel_for(_term_space.begin(), _term_space.end(), [&](const TermID t)
        {
            double gain = calc_info_gain(t, c);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if(feature_weights[t] < gain)
                    feature_weights[t] = gain;
            }
        });
    }

    return sort_terms(feature_weights);

}

double select_info_gain::calc_info_gain(TermID termID, const string & label)
{
    double p_tc = term_and_class(termID, label);
    double p_ntnc = not_term_and_not_class(termID, label);
    double p_ntc = not_term_and_class(termID, label);
    double p_tnc = not_term_and_class(termID, label);
    double p_c = _pclass[label];
    double p_t = _pterm[termID];
    double p_nc = 1.0 - p_c;
    double p_nt = 1.0 - p_t;

    double gain_tc = p_tc * log(p_tc / (p_t * p_c));
    double gain_ntnc = p_ntnc * log(p_ntnc / (p_nt * p_nc));
    double gain_ntc = p_ntc * log(p_ntc / (p_nt * p_c));
    double gain_tnc = p_tnc * log(p_tnc / (p_t * p_nc));

    return gain_tc + gain_ntnc + gain_ntc + gain_tnc;
}

}
}
