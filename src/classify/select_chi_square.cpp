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
    feature_select(docs) { /* nothing */ }

vector<pair<TermID, double>> select_chi_square::select()
{
    unordered_map<TermID, double> feature_weights;

    std::mutex _mutex;
    for(auto & c: _class_space)
    {
        parallel::parallel_for(_term_space.begin(), _term_space.end(), [&](const TermID t)
        {
            double chi = calc_chi_square(t, c);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if(feature_weights[t] < chi)
                    feature_weights[t] = chi;
            }
        });
    }

    vector<pair<TermID, double>> features(feature_weights.begin(), feature_weights.end());
    std::sort(features.begin(), features.end(),
        [](const pair<TermID, double> & a, const pair<TermID, double> & b) {
            return a.second > b.second;
        }
    );

    return features;
}

double select_chi_square::calc_chi_square(TermID termID, const string & label)
{
    double p_tc = term_and_class(termID, label);
    double p_ntnc = not_term_and_not_class(termID, label);
    double p_ntc = not_term_and_class(termID, label);
    double p_tnc = not_term_and_class(termID, label);
    double p_c = _pclass[label];
    double p_t = _pterm[termID];

    double numerator = p_tc * p_ntnc - p_ntc * p_tnc;
    numerator *= numerator;
    double denominator = p_c * (1.0 - p_c) * p_t * (1.0 - p_t);

    return numerator / denominator;
}

}
}
