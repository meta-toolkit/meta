/**
 * @file binary_classifier_factory.cpp
 * @author Chase Geigle
 */

#include "meta/classify/binary_classifier_factory.h"
#include "meta/classify/classifier/sgd.h"

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
                           binary_dataset_view training)
{
    auto id = config.get_as<std::string>("method");
    if (!id)
        throw binary_classifier_factory::exception{
            "method required in binary classifier configuration"};

    return binary_classifier_factory::get().create(*id, config,
                                                   std::move(training));
}

template <class Classifier>
void binary_classifier_loader::reg()
{
    add(Classifier::id, load_binary_classifier<Classifier>);
}

binary_classifier_loader::binary_classifier_loader()
{
    // built-in binary classifiers
    reg<sgd>();
}

std::unique_ptr<binary_classifier> load_binary_classifier(std::istream& in)
{
    std::string id;
    io::packed::read(in, id);
    return binary_classifier_loader::get().create(id, in);
}
}
}
