/**
 * @file okapi_bm25.cpp
 */

#include "index/okapi_bm25.h"

namespace meta {
namespace index {

okapi_bm25::okapi_bm25(double k1 /* = 1.5 */,
                       double b  /* = 0.75 */,
                       double k3 /* = 500.0 */):
    _k1(k1), _b(b), _k3(k3)
{ /* nothing */ }

std::vector<std::pair<doc_id, double>> okapi_bm25::score(inverted_index & idx,
                                                         document & query)
{
    idx.tokenize(query);
    std::unordered_map<doc_id, double> results;

    double avg_doc_length = idx.avg_doc_length();
    double num_docs = idx.num_docs();
    
    for(auto & t_pair: query.frequencies())
    {
        auto counts = idx.counts(t_pair.first);
        for(auto & d_pair: counts)
        {
            double doc_length = idx.doc_size(d_pair.first);
            // here we add 1.0 to the IDF to ensure that the result is positive
            double IDF = log(1.0 + (num_docs - counts.size() + 0.5)
                / (counts.size() + 0.5));
            double TF = ((_k1 + 1.0) * d_pair.second)
                / ((_k1 * ((1.0 - _b) + _b * doc_length / avg_doc_length)) + d_pair.second);
            double QTF = ((_k3 + 1.0) * t_pair.second)
                / (_k3 + t_pair.second);
            results[d_pair.first] += TF * IDF * QTF;
        }
    }

    std::vector<std::pair<doc_id, double>> final_results{results.begin(), results.end()};
    std::sort(final_results.begin(), final_results.end(),
        [](const std::pair<doc_id, double> & a, const std::pair<doc_id, double> & b) {
            return a.second > b.second;
        }
    );

    return final_results;
}

}
}
