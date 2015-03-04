/**
 * @file nearest_centroid.cpp
 * @author Sean Massung
 */

#include <vector>
#include <unordered_map>

#include "cpptoml.h"
#include "classify/classifier/nearest_centroid.h"
#include "corpus/document.h"
#include "index/postings_data.h"

namespace meta
{
namespace classify
{

const std::string nearest_centroid::id = "nearest-centroid";

nearest_centroid::nearest_centroid(std::shared_ptr<index::inverted_index> idx,
                                   std::shared_ptr<index::forward_index> f_idx)
    : classifier{std::move(f_idx)}, inv_idx_{std::move(idx)}
{ /* nothing */
}

void nearest_centroid::train(const std::vector<doc_id>& docs)
{
    double num_docs = idx_->num_docs();
    std::unordered_map<class_label, uint32_t> docs_per_class;

    // create document centroids based on averages of TF-IDF values

    for (auto& id : docs)
    {
        auto label = idx_->label(id);
        ++docs_per_class[label];
        for (auto& pair : idx_->search_primary(id)->counts())
        {
            term_id tid = pair.first;
            double count = pair.second;
            double tfidf = count * std::log(num_docs / inv_idx_->doc_freq(tid));
            centroids_[label][tid] += tfidf;
        }
    }

    for (auto& centroid : centroids_)
        for (auto& term_pair : centroid.second)
            term_pair.second /= docs_per_class[centroid.first];
}

class_label nearest_centroid::classify(doc_id d_id)
{
    double best_score = std::numeric_limits<double>::lowest();
    class_label best_label;

    auto pdata = idx_->search_primary(d_id);

    // convert to TF-IDF representation
    auto counts(pdata->counts());
    double num_docs = idx_->num_docs();
    for (auto& count : counts)
        count.second *= std::log(num_docs / inv_idx_->doc_freq(count.first));

    for (auto& centroid : centroids_)
    {
        double score = cosine_sim(counts, centroid.second);
        if (score > best_score)
        {
            best_score = score;
            best_label = centroid.first;
        }
    }

    return best_label;
}

double nearest_centroid::cosine_sim(
    const std::vector<std::pair<term_id, double>>& doc,
    const std::unordered_map<term_id, double>& centroid)
{
    double centroid_mag = 0.0;
    for (auto& pair : centroid)
        centroid_mag += pair.second * pair.second;

    double dot = 0.0;
    double doc_mag = 0.0;
    for (auto& count : doc)
    {
        doc_mag += count.second * count.second;
        auto it = centroid.find(count.first);
        if (it != centroid.end())
            dot += it->second * count.second;
    }

    centroid_mag = std::sqrt(centroid_mag);
    doc_mag = std::sqrt(doc_mag);
    return dot / (doc_mag * centroid_mag);
}

void nearest_centroid::reset()
{
    centroids_.clear();
}

template <>
std::unique_ptr<classifier> make_multi_index_classifier<nearest_centroid>(
    const cpptoml::table&, std::shared_ptr<index::forward_index> idx,
    std::shared_ptr<index::inverted_index> inv_idx)
{
    return make_unique<nearest_centroid>(std::move(inv_idx), std::move(idx));
}
}
}
