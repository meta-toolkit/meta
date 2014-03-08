/**
 * @file knn.cpp
 * @author Sean Massung
 */

#include <vector>
#include <unordered_map>

#include "cpptoml.h"
#include "classify/classifier/knn.h"
#include "corpus/document.h"
#include "index/postings_data.h"
#include "index/ranker/ranker_factory.h"

namespace meta {
namespace classify {

const std::string knn::id = "knn";

knn::knn(std::shared_ptr<index::inverted_index> idx,
                 std::shared_ptr<index::forward_index> f_idx, uint16_t k,
                 std::unique_ptr<index::ranker> ranker)
    : classifier{std::move(f_idx)},
      _inv_idx{std::move(idx)},
      _k{k},
      _ranker{std::move(ranker)}
{ /* nothing */ }

void knn::train(const std::vector<doc_id> & docs)
{
    _legal_docs.insert(docs.begin(), docs.end());
}

class_label knn::classify(doc_id d_id)
{
    if(_k > _legal_docs.size())
        throw knn_exception{"k must be smaller than the "
            "number of documents in the index (training documents)"};

    corpus::document query{"[no path]", d_id};
    for (const auto& count: _idx->search_primary(d_id)->counts())
        query.increment(_idx->term_text(count.first), count.second);

    auto scored =
        _ranker->score(*_inv_idx, query, _inv_idx->num_docs(), [&](doc_id d_id)
        {
            return _legal_docs.find(d_id) != _legal_docs.end();
        });

    std::unordered_map<class_label, uint16_t> counts;
    uint16_t i = 0;
    for(auto & s: scored)
    {
        ++counts[_idx->label(s.first)];
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

class_label knn::select_best_label(
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
        class_label lbl{_inv_idx->label(p.first)};
        auto f = best.find(lbl);
        if(f != best.end())
            return *f;
    }

    // suppress warnings
    return sorted.begin()->first;
}

void knn::reset()
{
    _legal_docs.clear();
}

template <>
std::unique_ptr<classifier> make_multi_index_classifier<knn>(
    const cpptoml::toml_group& config,
    std::shared_ptr<index::forward_index> idx,
    std::shared_ptr<index::inverted_index> inv_idx)
{
    auto k = config.get_as<int64_t>("k");
    if (!k)
        throw classifier_factory::exception{
            "knn requires k to be specified in its configuration"};
    auto ranker = config.get_group("ranker");
    if (!ranker)
        throw classifier_factory::exception{
            "knn requires a ranker to be specified in its configuration"};

    return make_unique<knn>(std::move(inv_idx), std::move(idx), *k,
                            index::make_ranker(*ranker));
}
}
}
