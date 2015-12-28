/**
 * @file one_vs_all.cpp
 * @author Chase Geigle
 */

#include "meta/classify/binary_classifier_factory.h"
#include "meta/classify/classifier/one_vs_all.h"
#include "meta/classify/classifier/online_binary_classifier.h"
#include "meta/parallel/parallel_for.h"

namespace meta
{
namespace classify
{

const util::string_view one_vs_all::id = "one-vs-all";

one_vs_all::one_vs_all(multiclass_dataset_view docs, const cpptoml::table& base)
{
    classifiers_.reserve(docs.total_labels());
    for (auto it = docs.labels_begin(), end = docs.labels_end(); it != end;
         ++it)
        classifiers_[it->first] = nullptr;

    parallel::parallel_for(
        classifiers_.begin(), classifiers_.end(),
        [&](std::pair<const class_label, std::unique_ptr<binary_classifier>>&
                pr)
        {
            binary_dataset_view bdv{docs, [&](const instance_type& instance)
                                    {
                                        return docs.label(instance) == pr.first;
                                    }};
            pr.second = make_binary_classifier(base, bdv);
        });
}

one_vs_all::one_vs_all(std::istream& in)
{
    auto size = io::packed::read<std::size_t>(in);
    classifiers_.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto lbl = io::packed::read<class_label>(in);
        classifiers_[lbl] = load_binary_classifier(in);
    }
}

void one_vs_all::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, classifiers_.size());
    for (const auto& pr : classifiers_)
    {
        io::packed::write(out, pr.first);
        pr.second->save(out);
    }
}

void one_vs_all::train(dataset_view_type docs)
{
    parallel::parallel_for(
        classifiers_.begin(), classifiers_.end(),
        [&](std::pair<const class_label, std::unique_ptr<binary_classifier>>&
                pr)
        {
            if (auto cls
                = dynamic_cast<online_binary_classifier*>(pr.second.get()))
            {
                binary_dataset_view bdv{docs, [&](const instance_type& instance)
                                        {
                                            return docs.label(instance)
                                                   == pr.first;
                                        }};

                cls->train(bdv);
            }
            else
            {
                throw classifier_exception{"base type in one_vs_all is not an "
                                           "online_binary_classifier"};
            }
        });
}

void one_vs_all::train_one(const feature_vector& doc, const class_label& label)
{
    for (const auto& pr : classifiers_)
    {
        if (auto cls = dynamic_cast<online_binary_classifier*>(pr.second.get()))
        {
            cls->train_one(doc, label == pr.first);
        }
        else
        {
            throw classifier_exception{
                "base type in one_vs_all is not an online_binary_classifier"};
        }
    }
}

class_label one_vs_all::classify(const feature_vector& doc) const
{
    class_label best_label;
    double best_prediction = std::numeric_limits<double>::lowest();
    for (auto& pair : classifiers_)
    {
        double prediction = pair.second->predict(doc);
        if (prediction > best_prediction)
        {
            best_prediction = prediction;
            best_label = pair.first;
        }
    }
    return best_label;
}

template <>
std::unique_ptr<classifier>
    make_classifier<one_vs_all>(const cpptoml::table& config,
                                multiclass_dataset_view training)
{
    auto base = config.get_table("base");
    if (!base)
        throw classifier_factory::exception{
            "one-vs-all missing base-classifier parameter in config file"};
    return make_unique<one_vs_all>(std::move(training), *base);
}
}
}
