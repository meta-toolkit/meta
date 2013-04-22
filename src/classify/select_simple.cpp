/**
 * @file select_simple.cpp
 */

#include "classify/select_simple.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::pair;
using index::document;

select_simple::select_simple(const vector<document> & docs):
    feature_select(docs) { /* nothing */ }

vector<pair<term_id, double>> select_simple::select()
{
    unordered_map<term_id, double> feature_weights;
    for(auto & c: _class_space)
    {
        for(auto & t: _term_space)
        {
            double weight = calc_weight(t, c);
            if(feature_weights[t] < weight)
                feature_weights[t] = weight;
        }
    }

    return sort_terms(feature_weights);
}

unordered_map<class_label, vector<pair<term_id, double>>> select_simple::select_by_class()
{
    unordered_map<class_label, vector<pair<term_id, double>>> features;

    for(auto & c: _class_space)
    {
        unordered_map<term_id, double> weights;
        for(auto & t: _term_space)
            weights[t] = calc_weight(t, c);

        features[c] = sort_terms(weights);
    }

    return features;
}

}
}
