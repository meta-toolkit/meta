/**
 * @file select_doc_freq.cpp
 */

#include <unordered_map>
#include <string>
#include "classify/select_doc_freq.h"
#include "classify/select.h"
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

select_doc_freq::select_doc_freq(const vector<Document> & docs):
    feature_select(docs) { /* nothing */ }

vector<pair<TermID, double>> select_doc_freq::select()
{
    unordered_map<TermID, double> feature_weights;

    std::mutex _mutex;
    for(auto & c: _class_space)
    {
        string progress = "  " + c + ": ";
        parallel::parallel_for(_term_space.begin(), _term_space.end(), [&](const TermID t)
        {
            double prob = term_and_class(t, c);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if(feature_weights[t] < prob)
                    feature_weights[t] = prob;
            }
        });
    }

    return sort_terms(feature_weights);
}

}
}
