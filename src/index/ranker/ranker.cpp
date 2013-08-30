/**
 * @file ranker.cpp
 * @author Sean Massung
 */

#include "index/ranker/ranker.h"

namespace meta {
namespace index {

std::vector<std::pair<doc_id, double>> ranker::score(inverted_index & idx,
                                                     document & query) const
{
    if(query.frequencies().empty())
        idx.tokenize(query);
    std::unordered_map<doc_id, double> results;

    for(auto & tpair: query.frequencies())
    {
        auto counts = idx.counts(tpair.first);
        for(auto & dpair: counts)
            results[dpair.first] +=
                score_one(idx, query, tpair, dpair, counts.size());
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
