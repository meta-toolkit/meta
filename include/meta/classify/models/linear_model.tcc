/**
 * @file linear_model.tcc
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include <cassert>
#include <fstream>

#include "meta/classify/models/linear_model.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/util/fixed_heap.h"

namespace meta
{
namespace classify
{

template <class FeatureId, class FeatureValue, class ClassId>
void linear_model<FeatureId, FeatureValue, ClassId>::load(std::istream& model)
{
    if (!model)
        throw exception{"model not found"};

    uint64_t num_feats;
    io::packed::read(model, num_feats);

    for (uint64_t i = 0; i < num_feats; ++i)
    {
        if (!model)
            throw exception{"malformed model file (too few features written)"};

        feature_id feature_name;
        io::packed::read(model, feature_name);

        uint64_t num_cids;
        io::packed::read(model, num_cids);

        for (uint64_t j = 0; j < num_cids; ++j)
        {
            if (!model)
                throw exception{"malformed model file (too few classes "
                                "written for feature)"};

            class_id cid;
            feature_value val;
            io::packed::read(model, cid);
            io::packed::read(model, val);

            weights_[feature_name][cid] = val;
        }
    }
}

template <class FeatureId, class FeatureValue, class ClassId>
void linear_model<FeatureId, FeatureValue, ClassId>::save(
    std::ostream& model) const
{
    uint64_t sze = weights_.size();
    io::packed::write(model, sze);
    for (const auto& feat_vec : weights_)
    {
        const auto& feat = feat_vec.first;
        const auto& weights = feat_vec.second;

        io::packed::write(model, feat);
        uint64_t size = weights.size();
        io::packed::write(model, size);

        for (const auto& weight : weights)
        {
            io::packed::write(model, weight.first);
            io::packed::write(model, weight.second);
        }
    }
}

template <class FeatureId, class FeatureValue, class ClassId>
template <class FeatureVector, class Filter>
auto linear_model<FeatureId, FeatureValue, ClassId>::best_class(
    FeatureVector&& features, Filter&& filter) const -> class_id
{
    weight_vector class_scores;
    for (const auto& feat : features)
    {
        const auto& name = feat.first;
        auto val = feat.second;

        auto it = weights_.find(name);
        if (it == weights_.end())
            continue;

        for (const auto& class_weight : it->second)
        {
            auto tid = class_weight.first;
            auto trans_w = class_weight.second;

            class_scores[tid] += val * trans_w;
        }
    }

    auto best_score = std::numeric_limits<feature_value>::lowest();
    class_id best_class{};
    for (const auto& score : class_scores)
    {
        auto cid = score.first;
        if (score.second > best_score && filter(cid))
        {
            best_class = score.first;
            best_score = score.second;
        }
    }

    return best_class;
}

template <class FeatureId, class FeatureValue, class ClassId>
template <class FeatureVector>
auto linear_model<FeatureId, FeatureValue, ClassId>::best_class(
    FeatureVector&& features) const -> class_id
{
    return best_class(std::forward<FeatureVector>(features), [](const class_id&)
                      {
        return true;
    });
}

template <class FeatureId, class FeatureValue, class ClassId>
template <class FeatureVector, class Filter>
auto linear_model<FeatureId, FeatureValue, ClassId>::best_classes(
    FeatureVector&& features, uint64_t num,
    Filter&& filter) const -> scored_classes
{
    weight_vector class_scores;
    for (const auto& feat : features)
    {
        const auto& name = feat.first;
        auto val = feat.second;

        auto it = weights_.find(name);
        if (it == weights_.end())
            continue;

        for (const auto& class_weight : it->second)
        {
            auto tid = class_weight.first;
            auto trans_w = class_weight.second;

            class_scores[tid] += val * trans_w;
        }
    }

    auto comp = [](const scored_class& lhs, const scored_class& rhs)
    {
        return lhs.second > rhs.second;
    };

    util::fixed_heap<scored_class, decltype(comp)> heap{num, comp};
    for (const auto& score : class_scores)
    {
        auto cid = score.first;
        if (filter(cid))
            heap.emplace(score);
    }

    return heap.extract_top();
}

template <class FeatureId, class FeatureValue, class ClassId>
template <class FeatureVector>
auto linear_model<FeatureId, FeatureValue, ClassId>::best_classes(
    FeatureVector&& features, uint64_t num) const -> scored_classes
{
    return best_classes(std::forward<FeatureVector>(features), num,
                        [](const class_id&)
                        {
        return true;
    });
}

template <class FeatureId, class FeatureValue, class ClassId>
void linear_model<FeatureId, FeatureValue, ClassId>::update(
    const weight_vectors& updates, feature_value scale)
{
    for (const auto& feat_vec : updates)
    {
        const auto& feat = feat_vec.first;
        auto& wv = weights_[feat];

        for (const auto& up : feat_vec.second)
            wv[up.first] += up.second * scale;
    }
}

template <class FeatureId, class FeatureValue, class ClassId>
void linear_model<FeatureId, FeatureValue, ClassId>::update(
    const class_id& cid, const feature_id& fid, feature_value delta)
{
    weights_[fid][cid] += delta;
}

template <class FeatureId, class FeatureValue, class ClassId>
void linear_model<FeatureId, FeatureValue, ClassId>::condense(bool log)
{
    // build feature set
    std::vector<feature_id> features;
    features.reserve(weights_.size());
    for (const auto& feat_vec : weights_)
        features.push_back(feat_vec.first);

    uint64_t nnz = 0;
    for (const auto& feat : features)
    {
        auto it = weights_.find(feat);
        it->second.condense();
        if (it->second.empty())
            weights_.erase(it);
        else
            nnz += it->second.size();
    }

    if (log)
    {
        LOG(info) << "Number of total features: " << weights_.size() << ENDLG;
        LOG(info) << "Number of nonzero weights: " << nnz << ENDLG;
    }
}

template <class FeatureId, class FeatureValue, class ClassId>
auto linear_model<FeatureId, FeatureValue, ClassId>::weights() const -> const
    weight_vectors &
{
    return weights_;
}
}
}
