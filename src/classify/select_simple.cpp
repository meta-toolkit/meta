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
using index::Document;

select_simple::select_simple(const vector<Document> & docs):
    feature_select(docs) { /* nothing */ }

vector<pair<TermID, double>> select_simple::select()
{
    unordered_map<TermID, double> feature_weights;
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

unordered_map<ClassLabel, vector<pair<TermID, double>>> select_simple::select_by_class()
{
    unordered_map<ClassLabel, vector<pair<TermID, double>>> features;

    for(auto & c: _class_space)
    {
        unordered_map<TermID, double> weights;
        for(auto & t: _term_space)
            weights[t] = calc_weight(t, c);

        features[c] = sort_terms(weights);
    }

    return features;
}

}
}
