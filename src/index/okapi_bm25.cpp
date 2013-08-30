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
    if(query.frequencies().empty())
        idx.tokenize(query);
    std::unordered_map<doc_id, double> results;

    double avg_dl = idx.avg_doc_length();
    double num_docs = idx.num_docs();
    
    for(auto & t_pair: query.frequencies())
    {
        auto counts = idx.counts(t_pair.first);
        for(auto & dpair: counts)
        {
            double doc_len = idx.doc_size(dpair.first);
            // here we add 1.0 to the IDF to ensure that the result is positive
            double IDF = log(1.0 + (num_docs - counts.size() + 0.5)
                / (counts.size() + 0.5));
            double TF = ((_k1 + 1.0) * dpair.second)
                / ((_k1 * ((1.0 - _b) + _b * doc_len / avg_dl)) + dpair.second);
            double QTF = ((_k3 + 1.0) * t_pair.second)
                / (_k3 + t_pair.second);
            results[dpair.first] += TF * IDF * QTF;
        }
    }

    using doc_pair = std::pair<doc_id, double>;
    std::vector<doc_pair> final_results{results.begin(), results.end()};
    std::sort(final_results.begin(), final_results.end(),
        [](const doc_pair & a, const doc_pair & b) {
            return a.second > b.second;
        }
    );

    return final_results;
}

}
}
