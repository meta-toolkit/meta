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
void classifier_factory::reg()
{
    add(Classifier::id, [](const cpptoml::table& config,
                           std::shared_ptr<index::forward_index> idx,
                           std::shared_ptr<index::inverted_index>)
        {
        return make_classifier<Classifier>(config, std::move(idx));
    });
}

template <class Classifier>
void classifier_factory::reg_mi()
{
    add(Classifier::id, make_multi_index_classifier<Classifier>);
}

classifier_factory::classifier_factory()
{
    // built-in classifiers
    reg<one_vs_all>();
    reg<one_vs_one>();
    reg<naive_bayes>();
    reg<svm_wrapper>();
    reg<winnow>();
    reg<dual_perceptron>();
    reg<logistic_regression>();

    // built-in multi-index classifiers
    reg_mi<knn>();
    reg_mi<nearest_centroid>();
}

std::unique_ptr<classifier> make_classifier(
    const cpptoml::table& config, std::shared_ptr<index::forward_index> idx,
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
