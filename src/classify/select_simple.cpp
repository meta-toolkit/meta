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
using std::string;
using std::pair;

using index::TermID;
using index::Document;

select_simple::select_simple(const vector<Document> & docs):
    feature_select(docs) { /* nothing */ }

vector<pair<TermID, double>> select_simple::select()
{
    unordered_map<TermID, double> feature_weights;

    std::mutex _mutex;
    for(auto & c: _class_space)
    {
        parallel::parallel_for(_term_space.begin(), _term_space.end(), [&](const TermID t)
        {
            double weight = calc_weight(t, c);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if(feature_weights[t] < weight)
                    feature_weights[t] = weight;
            }
        });
    }

    return sort_terms(feature_weights);
}

}
}
