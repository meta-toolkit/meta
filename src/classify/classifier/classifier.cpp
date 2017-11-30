/**
 * @file classifier.cpp
 * @author Sean Massung
 */

#include <random>
#include <numeric>
#include "meta/logging/logger.h"
#include "meta/classify/classifier/classifier.h"
#include "meta/classify/classifier_factory.h"

namespace meta
{
namespace classify
{

confusion_matrix classifier::test(dataset_view_type docs) const
{

    confusion_matrix matrix;
    for (const auto& instance : docs)
        matrix.add(predicted_label{classify(instance.weights)},
                   docs.label(instance));

    return matrix;
}

confusion_matrix cross_validate(const cpptoml::table& config,
                                classifier::dataset_view_type docs, size_t k,
                                bool even_split /* = false */)
{
    return cross_validate(
        [&](multiclass_dataset_view fold)
        {
            return make_classifier(config, std::move(fold));
        },
        std::move(docs), k, even_split);
}
}
}
