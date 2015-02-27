/**
 * @file binary_classifier_factory.cpp
 * @author Chase Geigle
 */

#include "classify/binary_classifier_factory.h"
#include "classify/classifier/sgd.h"

namespace meta
{
namespace classify
{

template <class Classifier>
void binary_classifier_factory::reg()
{
    add(Classifier::id, make_binary_classifier<Classifier>);
}

binary_classifier_factory::binary_classifier_factory()
{
    // built-in binary classifiers
    reg<sgd>();
}

std::unique_ptr<binary_classifier>
    make_binary_classifier(const cpptoml::table& config,
                           std::shared_ptr<index::forward_index> idx,
                           class_label positive, class_label negative)
{
    auto id = config.get_as<std::string>("method");
    if (!id)
        throw binary_classifier_factory::exception{
            "method required in binary classifier configuration"};

    return binary_classifier_factory::get().create(*id, config, std::move(idx),
                                                   positive, negative);
}
}
}
