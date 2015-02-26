/**
 * @file one_vs_one.cpp
 * @author Chase Geigle
 */

#include <iterator>

#include "classify/binary_classifier_factory.h"
#include "classify/classifier/one_vs_one.h"
#include "parallel/parallel_for.h"

namespace meta
{
namespace classify
{

const std::string one_vs_one::id = "one-vs-one";

void one_vs_one::train(const std::vector<doc_id>& docs)
{
    std::unordered_map<class_label, std::vector<doc_id>> partitions;
    for (const auto& id : docs)
        partitions[idx_->label(id)].emplace_back(id);

    parallel::parallel_for(classifiers_.begin(), classifiers_.end(),
                           [&](const std::unique_ptr<binary_classifier>& p)
                           {
        const auto& pos = partitions[p->positive_label()];
        const auto& neg = partitions[p->negative_label()];

        std::vector<doc_id> examples;
        examples.reserve(pos.size() + neg.size());

        std::copy(pos.begin(), pos.end(), std::back_inserter(examples));
        std::copy(neg.begin(), neg.end(), std::back_inserter(examples));

        p->train(examples);
    });
}

class_label one_vs_one::classify(doc_id d_id)
{
    std::unordered_map<class_label, int> votes;
    std::mutex mut;

    parallel::parallel_for(classifiers_.begin(), classifiers_.end(),
                           [&](const std::unique_ptr<binary_classifier>& p)
                           {
        auto lbl = p->classify(d_id);
        std::lock_guard<std::mutex> lock{mut};
        votes[lbl]++;
    });

    using count_type = std::pair<const class_label, int>;
    auto iter
        = std::max_element(votes.begin(), votes.end(),
                           [](const count_type& lhs, const count_type& rhs)
                           {
            return lhs.second < rhs.second;
        });
    return iter->first;
}

void one_vs_one::reset()
{
    for (auto& p : classifiers_)
        p->reset();
}

template <>
std::unique_ptr<classifier>
    make_classifier<one_vs_one>(const cpptoml::table& config,
                                std::shared_ptr<index::forward_index> idx)
{
    auto base = config.get_table("base");
    if (!base)
        throw classifier_factory::exception{
            "one-vs-all missing base-classifier parameter in config file"};
    auto create = [&](class_label positive_label, class_label negative_label)
    {
        return make_binary_classifier(*base, idx, positive_label,
                                      negative_label);
    };
    return make_unique<one_vs_one>(idx, create);
}
}
}
