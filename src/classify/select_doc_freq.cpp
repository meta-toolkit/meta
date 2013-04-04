/**
 * @file select_doc_freq.cpp
 */

#include <unordered_map>
#include <string>
#include "classify/select_doc_freq.h"
#include "classify/select.h"
#include "util/common.h"
#include "parallel/parallel_for.h"

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

    std::mutex _mutex;
    for(auto & c: classes)
    {
        string progress = "  " + c.first + ": ";
        size_t i = 0;
        parallel::parallel_for(feature_space.begin(), feature_space.end(), [&](const TermID t)
        {
            double prob = term_given_class(t, c.first, classes);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                common::show_progress(i++, feature_space.size(), 50, progress);
                if(feature_weights[t] < prob)
                    feature_weights[t] = prob;
            }
        });
        common::end_progress(progress);
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
