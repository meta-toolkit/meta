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
#include "index/ranker/ranker_factory.h"

namespace meta
{
namespace classify
{

const std::string nearest_centroid::id = "nearest-centroid";

nearest_centroid::nearest_centroid(std::shared_ptr<index::inverted_index> idx,
                                   std::shared_ptr<index::forward_index> f_idx,
                                   std::unique_ptr<index::ranker> ranker,
                                   bool weighted /* = false */)
    : classifier{std::move(f_idx)},
      inv_idx_{std::move(idx)},
      ranker_{std::move(ranker)},
      weighted_{weighted}
{ /* nothing */
}

void nearest_centroid::train(const std::vector<doc_id>& docs)
{
}

class_label nearest_centroid::classify(doc_id d_id)
{
    return class_label{"TBA"};
}

void nearest_centroid::reset()
{
}

template <>
std::unique_ptr<classifier> make_multi_index_classifier<nearest_centroid>(
    const cpptoml::toml_group& config,
    std::shared_ptr<index::forward_index> idx,
    std::shared_ptr<index::inverted_index> inv_idx)
{
    auto ranker = config.get_group("ranker");
    if (!ranker)
        throw classifier_factory::exception{"nearest_centroid requires a "
                                            "ranker to be specified in its "
                                            "configuration"};

    bool use_weighted = false;
    auto weighted = config.get_as<bool>("weighted");
    if (weighted)
        use_weighted = *weighted;

    return make_unique<nearest_centroid>(std::move(inv_idx), std::move(idx),
                                         index::make_ranker(*ranker),
                                         use_weighted);
}
}
}
