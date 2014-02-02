/**
 * @file knn.cpp
 * @author Sean Massung
 */

#include <vector>
#include <unordered_map>
#include "classify/classifier/knn.h"
#include "corpus/document.h"

namespace meta {
namespace classify {

template <class Ranker>
template <class... Args>
knn<Ranker>::knn(index::inverted_index & idx, uint16_t k, Args &&... args):
    classifier{idx},
    _k{k},
    _ranker{std::forward<Args>(args)...}
{ /* nothing */ }

template <class Ranker>
void knn<Ranker>::train(const std::vector<doc_id> & docs)
{
    _legal_docs.insert(docs.begin(), docs.end());
}

template <class Ranker>
class_label knn<Ranker>::classify(doc_id d_id)
{
    if(_k > _legal_docs.size())
        throw knn_exception{"k must be smaller than the "
            "number of documents in the index (training documents)"};

    corpus::document query{_idx.doc_path(d_id), d_id};
    auto scored = _ranker.score(_idx, query, _idx.num_docs(), [&](doc_id d_id) {
        return _legal_docs.find(d_id) != _legal_docs.end();
    });

    std::unordered_map<class_label, uint16_t> counts;
    uint16_t i = 0;
    for(auto & s: scored)
    {
        ++counts[_idx.label(s.first)];
        if(++i > _k)
            break;
    }

    if(counts.empty())
        throw knn_exception{"label counts were empty"};

    using pair_t = std::pair<class_label, uint16_t>;
    std::vector<pair_t> sorted{counts.begin(), counts.end()};
    std::sort(sorted.begin(), sorted.end(),
        [](const pair_t & a, const pair_t & b) {
            return a.second > b.second;
        }
    );

    return select_best_label(scored, sorted);
}

template <class Ranker>
class_label knn<Ranker>::select_best_label(
    const std::vector<std::pair<doc_id, double>> & scored,
    const std::vector<std::pair<class_label, uint16_t>> & sorted) const
{
    uint16_t highest = sorted.begin()->second;
    auto it = sorted.begin();
    std::unordered_set<class_label> best;
    while(it != sorted.end() && it->second == highest)
    {
        best.insert(it->first);
        ++it;
    }

    // now best contains all the class labels that are tied for the highest vote

    if(best.size() == 1)
        return *best.begin();

    // since there is a tie, return the class label that appeared first in the
    // rankings

    for(auto & p: scored)
    {
        class_label lbl{_idx.label(p.first)};
        auto f = best.find(lbl);
        if(f != best.end())
            return *f;
    }

    // suppress warnings
    return sorted.begin()->first;
}

template <class Ranker>
void knn<Ranker>::reset()
{
    _legal_docs.clear();
}

}
}
