/**
 * @file nearest_centroid.cpp
 * @author Sean Massung
 */

#include <vector>
#include <unordered_map>

#include "cpptoml.h"
#include "meta/classify/classifier/nearest_centroid.h"
#include "meta/corpus/document.h"
#include "meta/index/postings_data.h"

namespace meta
{
namespace classify
{

const util::string_view nearest_centroid::id = "nearest-centroid";

nearest_centroid::nearest_centroid(multiclass_dataset_view docs,
                                   std::shared_ptr<index::inverted_index> idx)
    : inv_idx_{std::move(idx)}
{
    double num_docs = inv_idx_->num_docs();
    std::unordered_map<class_label, uint32_t> docs_per_class;

    // create document centroids based on averages of TF-IDF values

    for (const auto& instance : docs)
    {
        const auto& label = docs.label(instance);
        ++docs_per_class[label];
        for (auto& pair : instance.weights)
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

nearest_centroid::nearest_centroid(std::istream& in)
{
    // hackily load in the index from its stored path
    auto path = io::packed::read<std::string>(in);
    auto config = cpptoml::parse_file(path + "/config.toml");
    inv_idx_ = index::make_index<index::inverted_index>(*config);

    auto size = io::packed::read<std::size_t>(in);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto lbl = io::packed::read<class_label>(in);
        auto& map_ref = centroids_[lbl];

        auto isize = io::packed::read<std::size_t>(in);
        for (std::size_t j = 0; j < isize; ++j)
        {
            auto id = io::packed::read<term_id>(in);
            auto weight = io::packed::read<double>(in);
            map_ref.emplace(id, weight);
        }
    }
}

void nearest_centroid::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, inv_idx_->index_name());

    io::packed::write(out, centroids_.size());
    for (const auto& pr : centroids_)
    {
        io::packed::write(out, pr.first);
        io::packed::write(out, pr.second.size());
        for (const auto& ipr : pr.second)
        {
            io::packed::write(out, ipr.first);
            io::packed::write(out, ipr.second);
        }
    }
}

class_label nearest_centroid::classify(const feature_vector& instance) const
{
    double best_score = std::numeric_limits<double>::lowest();
    class_label best_label;

    // convert to TF-IDF representation
    double num_docs = inv_idx_->num_docs();
    auto counts(instance);
    for (auto& count : counts)
        count.second *= std::log(num_docs / inv_idx_->doc_freq(count.first));

    for (auto& centroid : centroids_)
    {
        double score
            = cosine_sim(counts.begin(), counts.end(), centroid.second);
        if (score > best_score)
        {
            best_score = score;
            best_label = centroid.first;
        }
    }

    return best_label;
}

template <class ForwardIterator>
double nearest_centroid::cosine_sim(
    ForwardIterator begin, ForwardIterator end,
    const std::unordered_map<term_id, double>& centroid) const
{
    double centroid_mag = 0.0;
    for (const auto& pair : centroid)
        centroid_mag += pair.second * pair.second;

    double dot = 0.0;
    double doc_mag = 0.0;
    for (; begin != end; ++begin)
    {
        doc_mag += begin->second * begin->second;
        auto it = centroid.find(begin->first);
        if (it != centroid.end())
            dot += it->second * begin->second;
    }

    centroid_mag = std::sqrt(centroid_mag);
    doc_mag = std::sqrt(doc_mag);
    return dot / (doc_mag * centroid_mag);
}

template <>
std::unique_ptr<classifier> make_multi_index_classifier<nearest_centroid>(
    const cpptoml::table&, multiclass_dataset_view training,
    std::shared_ptr<index::inverted_index> inv_idx)
{
    return make_unique<nearest_centroid>(std::move(training),
                                         std::move(inv_idx));
}
}
}
