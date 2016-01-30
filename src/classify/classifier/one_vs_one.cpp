/**
 * @file one_vs_one.cpp
 * @author Chase Geigle
 */

#include <iterator>

#include "meta/classify/binary_classifier_factory.h"
#include "meta/classify/classifier/one_vs_one.h"
#include "meta/classify/classifier/online_binary_classifier.h"
#include "meta/parallel/parallel_for.h"

namespace meta
{
namespace classify
{

const util::string_view one_vs_one::id = "one-vs-one";

one_vs_one::one_vs_one(multiclass_dataset_view docs, const cpptoml::table& base)
{
    // for deterministic results, we create a list of the possible class
    // labels and sort them so that we always create the same pairs for the
    // one-vs-one reduction. This matters when using e.g. SGD where the
    // termination depends on the loss, which depends on which class we
    // treat as positive in the reduction.
    {
        std::vector<class_label> labels;
        labels.reserve(docs.total_labels());
        for (auto it = docs.labels_begin(); it != docs.labels_end(); ++it)
            labels.push_back(it->first);
        std::sort(labels.begin(), labels.end());

        for (auto outer = labels.begin(), end = labels.end(); outer != end;
             ++outer)
        {
            for (auto inner = outer; ++inner != end;)
            {
                classifiers_.emplace(problem_type{*outer, *inner}, nullptr);
            }
        }
    }

    using size_type = multiclass_dataset_view::size_type;
    using indices_type = std::vector<size_type>;

    // partition by class label
    std::unordered_map<class_label, indices_type> partitions;
    for (auto it = docs.begin(), end = docs.end(); it != end; ++it)
        partitions[docs.label(*it)].push_back(it.index());

    parallel::parallel_for(
        classifiers_.begin(), classifiers_.end(),
        [&](classifier_map_type::value_type& problem)
        {
            const auto& pos = partitions[problem.first.positive];
            const auto& neg = partitions[problem.first.negative];

            indices_type indices;
            indices.reserve(pos.size() + neg.size());
            std::copy(pos.begin(), pos.end(), std::back_inserter(indices));
            std::copy(neg.begin(), neg.end(), std::back_inserter(indices));

            binary_dataset_view bdv{
                docs, std::move(indices), [&](const instance_type& instance)
                {
                    return docs.label(instance) == problem.first.positive;
                }};

            problem.second = make_binary_classifier(base, bdv);
        });

#ifndef NDEBUG
    for (const auto& pr : classifiers_)
        assert(pr.second != nullptr);
#endif
}

one_vs_one::one_vs_one(std::istream& in)
{
    auto size = io::packed::read<std::size_t>(in);
    classifiers_.reserve(size);

    for (std::size_t i = 0; i < size; ++i)
    {
        auto pos = io::packed::read<class_label>(in);
        auto neg = io::packed::read<class_label>(in);

        classifiers_.emplace(problem_type{pos, neg},
                             load_binary_classifier(in));
    }
}

void one_vs_one::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, classifiers_.size());
    for (const auto& pr : classifiers_)
    {
        io::packed::write(out, pr.first.positive);
        io::packed::write(out, pr.first.negative);
        pr.second->save(out);
    }
}

void one_vs_one::train(dataset_view_type docs)
{
    using size_type = multiclass_dataset_view::size_type;
    using indices_type = std::vector<size_type>;

    // partition by class label
    std::unordered_map<class_label, indices_type> partitions;
    for (auto it = docs.begin(), end = docs.end(); it != end; ++it)
        partitions[docs.label(*it)].push_back(it.index());

    parallel::parallel_for(
        classifiers_.begin(), classifiers_.end(),
        [&](classifier_map_type::value_type& problem)
        {
            const auto& pos = partitions[problem.first.positive];
            const auto& neg = partitions[problem.first.negative];

            indices_type indices;
            indices.reserve(pos.size() + neg.size());
            std::copy(pos.begin(), pos.end(), std::back_inserter(indices));
            std::copy(neg.begin(), neg.end(), std::back_inserter(indices));

            if (auto cls
                = dynamic_cast<online_binary_classifier*>(problem.second.get()))
            {

                binary_dataset_view bdv{
                    docs, std::move(indices), [&](const instance_type& instance)
                    {
                        return docs.label(instance) == problem.first.positive;
                    }};
                cls->train(bdv);
            }
            else
            {
                throw classifier_exception{"base type in one_vs_one is not an "
                                           "online_binary_classifier"};
            }
        });
}

void one_vs_one::train_one(const feature_vector& doc, const class_label& label)
{
    for (const auto& problem : classifiers_)
    {
        if (problem.first.positive == label || problem.first.negative == label)
        {
            if (auto cls
                = dynamic_cast<online_binary_classifier*>(problem.second.get()))
            {
                cls->train_one(doc, label == problem.first.positive);
            }
            else
            {
                throw classifier_exception{"base type in one_vs_one is not an "
                                           "online_binary_classifier"};
            }
        }
    }
}

class_label one_vs_one::classify(const feature_vector& instance) const
{
    std::unordered_map<class_label, int> votes;
    std::mutex mut;

    for (const auto& val : classifiers_)
    {
        auto lbl = val.second->classify(instance);
        if (lbl)
            ++votes[val.first.positive];
        else
            ++votes[val.first.negative];
    }

    using count_type = std::pair<const class_label, int>;
    auto iter = std::max_element(
        votes.begin(), votes.end(),
        [](const count_type& lhs, const count_type& rhs)
        {
            // a tie-breaking sort: if two classes have the same number of
            // votes, arbitrarily pick the one that comes alphabetically
            // before the other. This is mainly here for deterministic
            // results.
            return lhs.second < rhs.second
                   || (lhs.second == rhs.second && lhs.first < rhs.first);
        });
    return iter->first;
}

template <>
std::unique_ptr<classifier>
make_classifier<one_vs_one>(const cpptoml::table& config,
                            multiclass_dataset_view training)
{
    auto base = config.get_table("base");
    if (!base)
        throw classifier_factory::exception{
            "one-vs-all missing base-classifier parameter in config file"};
    return make_unique<one_vs_one>(training, *base);
}
}
}
