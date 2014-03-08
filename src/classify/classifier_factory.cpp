/**
 * @file classifier_factory.cpp
 * @author Chase Geigle
 */

#include "cpptoml.h"
#include "classify/classifier/all.h"
#include "classify/classifier_factory.h"

namespace meta
{
namespace classify
{

template <class Classifier>
void classifier_factory::add()
{
    add(Classifier::id, [](const cpptoml::toml_group& config,
                           std::shared_ptr<index::forward_index> idx,
                           std::shared_ptr<index::inverted_index>)
    {
        return make_classifier<Classifier>(config, std::move(idx));
    });
}

template <class Classifier>
void classifier_factory::add_mi()
{
    add(Classifier::id, make_multi_index_classifier<Classifier>);
}

classifier_factory::classifier_factory()
{
    // built-in classifiers
    add<one_vs_all>();
    add<naive_bayes>();
    add<svm_wrapper>();
    add<winnow>();
    add<dual_perceptron>();

    // built-in multi-index classifiers
    add_mi<knn>();
}

auto classifier_factory::create(const std::string& identifier,
                                const cpptoml::toml_group& config,
                                std::shared_ptr<index::forward_index> idx,
                                std::shared_ptr<index::inverted_index> inv_idx)
    -> pointer
{
    if (methods_.find(identifier) == methods_.end())
        throw exception{"unrecognized classifier id"};
    return methods_[identifier](config, std::move(idx), std::move(inv_idx));
}

std::unique_ptr<classifier> make_classifier(
    const cpptoml::toml_group& config,
    std::shared_ptr<index::forward_index> idx,
    std::shared_ptr<index::inverted_index> inv_idx /*= nullptr*/)
{
    auto id = config.get_as<std::string>("method");
    if (!id)
        throw classifier_factory::exception{
            "method required in classifier configuration"};
    return classifier_factory::get().create(*id, config, std::move(idx),
                                            std::move(inv_idx));
}
}
}
