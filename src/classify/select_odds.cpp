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
using std::string;
using std::pair;

using index::TermID;
using index::Document;

select_odds_ratio::select_odds_ratio(const vector<Document> & docs):
    feature_select(docs) { /* nothing */ }

vector<pair<TermID, double>> select_odds_ratio::select()
{
    unordered_map<TermID, double> feature_weights;

    std::mutex _mutex;
    for(auto & c: _class_space)
    {
        parallel::parallel_for(_term_space.begin(), _term_space.end(), [&](const TermID t)
        {
            double odds = calc_odds_ratio(t, c);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if(feature_weights[t] < odds)
                    feature_weights[t] = odds;
            }
        });
    }

    return sort_terms(feature_weights);
}

double select_odds_ratio::calc_odds_ratio(TermID termID, const string & label)
{
    double p_tc = term_and_class(termID, label);
    double p_tnc = term_and_not_class(termID, label);
    double denominator = (1.0 - p_tc) * p_tnc;

    if(denominator == 0.0)
        return 0.0;

    double numerator = p_tc * (1.0 - p_tnc);

    return log(numerator / denominator);
}

}
}
