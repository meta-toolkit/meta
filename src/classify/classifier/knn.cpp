/**
 * @file knn.cpp
 * @author Sean Massung
 */

#include <vector>
#include <unordered_map>

#include "cpptoml.h"
#include "meta/classify/classifier/knn.h"
#include "meta/corpus/document.h"
#include "meta/index/postings_data.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/index/make_index.h"

namespace meta
{
namespace classify
{

const util::string_view knn::id = "knn";

knn::knn(multiclass_dataset_view docs,
         std::shared_ptr<index::inverted_index> idx, uint16_t k,
         std::unique_ptr<index::ranker> ranker, bool weighted /* = false */)
    : inv_idx_{std::move(idx)},
      k_{k},
      ranker_{std::move(ranker)},
      weighted_{weighted}
{
    legal_docs_.reserve(docs.size());
    for (const auto& instance : docs)
        legal_docs_.insert(doc_id(instance.id));
}

knn::knn(std::istream& in)
    : weighted_{io::packed::read<bool>(in)}
{
    // hackily load in the index from its stored path
    auto path = io::packed::read<std::string>(in);
    auto config = cpptoml::parse_file(path + "/config.toml");
    inv_idx_ = index::make_index<index::inverted_index>(*config);

    io::packed::read(in, k_);
    ranker_ = index::load_ranker(in);

    auto size = io::packed::read<std::size_t>(in);
    legal_docs_.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto id = io::packed::read<doc_id>(in);
        legal_docs_.insert(id);
    }
}

void knn::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, weighted_);
    io::packed::write(out, inv_idx_->index_name());
    io::packed::write(out, k_);
    ranker_->save(out);

    io::packed::write(out, legal_docs_.size());
    for (const auto& doc : legal_docs_)
        io::packed::write(out, doc);
}

class_label knn::classify(const feature_vector& instance) const
{
    if (k_ > legal_docs_.size())
        throw knn_exception{
            "k must be smaller than the "
            "number of documents in the index (training documents)"};

    analyzers::feature_map<uint64_t> query{instance.size()};
    for (const auto& count : instance)
        query[inv_idx_->term_text(count.first)] += count.second;
    assert(query.size() > 0);

    auto scored = ranker_->score(
        *inv_idx_, query.begin(), query.end(), k_, [&](doc_id d_id)
        {
            return legal_docs_.find(d_id) != legal_docs_.end();
        });

    std::unordered_map<class_label, double> counts;
    for (auto& s : scored)
    {
        // normally, weighted k-nn weights neighbors by 1/distance, but since
        // our scores are similarity scores, we weight by the similarity
        if (weighted_)
            counts[inv_idx_->label(s.d_id)] += s.score;
        // if not weighted, each neighbor gets an equal vote
        else
            ++counts[inv_idx_->label(s.d_id)];
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
    const std::vector<index::search_result>& scored,
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
    for (auto& result : scored)
    {
        class_label lbl{inv_idx_->label(result.d_id)};
        auto f = best.find(lbl);
        if (f != best.end())
            return *f;
    }

    // suppress warnings
    return sorted.begin()->first;
}

template <>
std::unique_ptr<classifier> make_multi_index_classifier<knn>(
    const cpptoml::table& config, multiclass_dataset_view training,
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

    auto use_weighted = config.get_as<bool>("weighted").value_or(false);
    return make_unique<knn>(std::move(training), std::move(inv_idx), *k,
                            index::make_ranker(*ranker), use_weighted);
}
}
}
