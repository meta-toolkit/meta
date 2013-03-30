/**
 * @file select_doc_freq.cpp
 */

#include <unordered_map>
#include <string>
#include "classify/select_doc_freq.h"
#include "classify/select.h"
#include "util/common.h"

namespace meta {
namespace classify {

using std::string;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::vector;

using index::Document;
using index::TermID;

vector<pair<TermID, double>> feature_select::doc_freq(const vector<Document> & docs)
{
    unordered_map<string, vector<Document>> classes(partition_classes(docs));
    unordered_map<TermID, double> feature_weights;
    unordered_set<TermID> feature_space(get_term_space(docs));

    for(auto & c: classes)
    {
        size_t i = 0;
        for(auto & term: feature_space)
        {
            common::show_progress(i++, feature_space.size(), 100, "  " + c.first + ": ");
            double prob = term_given_class(term, c.first, classes);
            if(feature_weights[term] < prob)
                feature_weights[term] = prob;
        }
        common::end_progress("  " + c.first + ": ");
    }

    vector<pair<TermID, double>> features(feature_weights.begin(), feature_weights.end());
    std::sort(features.begin(), features.end(),
        [](const pair<TermID, double> & a, const pair<TermID, double> & b) {
            return a.second > b.second;
        }
    );

    return features;
}

}
}
