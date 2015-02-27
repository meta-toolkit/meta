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

namespace meta
{
namespace classify
{

const std::string knn::id = "knn";

knn::knn(std::shared_ptr<index::inverted_index> idx,
         std::shared_ptr<index::forward_index> f_idx, uint16_t k,
         std::unique_ptr<index::ranker> ranker, bool weighted /* = false */)
    : classifier{std::move(f_idx)},
      inv_idx_{std::move(idx)},
      k_{k},
      ranker_{std::move(ranker)},
      weighted_{weighted}
{ /* nothing */
}

void knn::train(const std::vector<doc_id>& docs)
{
    legal_docs_.insert(docs.begin(), docs.end());
}

class_label knn::classify(doc_id d_id)
{
    if (k_ > legal_docs_.size())
        throw knn_exception{
            "k must be smaller than the "
            "number of documents in the index (training documents)"};

    corpus::document query{"[no path]", d_id};
    for (const auto& count : idx_->search_primary(d_id)->counts())
        query.increment(idx_->term_text(count.first), count.second);

    auto scored = ranker_->score(*inv_idx_, query, inv_idx_->num_docs(),
                                 [&](doc_id d_id)
                                 {
        return legal_docs_.find(d_id) != legal_docs_.end();
    });

    std::unordered_map<class_label, double> counts;
    uint16_t i = 0;
    for (auto& s : scored)
    {
        // normally, weighted k-nn weights neighbors by 1/distance, but since
        // our scores are similarity scores, we weight by the similarity
        if (weighted_)
            counts[idx_->label(s.first)] += s.second;
        // if not weighted, each neighbor gets an equal vote
        else
            ++counts[idx_->label(s.first)];

        if (++i > k_)
            break;
    }

    if (counts.empty())
        throw knn_exception{"label counts were empty"};

    using pair_t = std::pair<class_label, uint16_t>;
    std::vector<pair_t> sorted{counts.begin(), counts.end()};
    std::sort(sorted.begin(), sorted.end(), [](const pair_t& a, const pair_t& b)
              {
        return a.second > b.second;
    });

    return select_best_label(scored, sorted);
}

class_label knn::select_best_label(
    const std::vector<std::pair<doc_id, double>>& scored,
    const std::vector<std::pair<class_label, uint16_t>>& sorted) const
{
    uint16_t highest = sorted.begin()->second;
    auto it = sorted.begin();
    std::unordered_set<class_label> best;
    while (it != sorted.end() && it->second == highest)
    {
        best.insert(it->first);
        ++it;
    }

    // now best contains all the class labels that are tied for the highest vote
    if (best.size() == 1)
        return *best.begin();

    // since there is a tie, return the class label that appeared first in the
    // rankings; this will usually only happen if the neighbor scores are not
    // weighted
    for (auto& p : scored)
    {
        class_label lbl{inv_idx_->label(p.first)};
        auto f = best.find(lbl);
        if (f != best.end())
            return *f;
    }

    // suppress warnings
    return sorted.begin()->first;
}

void knn::reset()
{
    legal_docs_.clear();
}

template <>
std::unique_ptr<classifier> make_multi_index_classifier<knn>(
    const cpptoml::table& config, std::shared_ptr<index::forward_index> idx,
    std::shared_ptr<index::inverted_index> inv_idx)
{
    auto k = config.get_as<int64_t>("k");
    if (!k)
        throw classifier_factory::exception{
            "knn requires k to be specified in its configuration"};

    auto ranker = config.get_table("ranker");
    if (!ranker)
        throw classifier_factory::exception{
            "knn requires a ranker to be specified in its configuration"};

    bool use_weighted = false;
    auto weighted = config.get_as<bool>("weighted");
    if (weighted)
        use_weighted = *weighted;

    return make_unique<knn>(std::move(inv_idx), std::move(idx), *k,
                            index::make_ranker(*ranker), use_weighted);
}
}
}
