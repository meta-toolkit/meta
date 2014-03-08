/**
 * @file binary_classifier_factory.h
 * @author Chase Geigle
 */

#ifndef _META_BINARY_CLASSIFIER_FACTORY_H_
#define _META_BINARY_CLASSIFIER_FACTORY_H_

#include "classify/classifier/binary_classifier.h"
#include "util/factory.h"

namespace cpptoml
{
class toml_group;
}

namespace meta
{
namespace classify
{

/**
 * Factory that is responsible for creating binary classifiers from
 * configuration files. Clients should use the register_binary_classifier
 * method instead of this class directly to add their own binary
 * classifiers.
 */
class binary_classifier_factory
    : public util::factory<binary_classifier_factory, binary_classifier,
                           const cpptoml::toml_group&,
                           std::shared_ptr<index::forward_index>, class_label,
                           class_label>
{
    friend base_factory;

  private:
    binary_classifier_factory();

    template <class Classifier>
    void reg();
};

/**
 * Convenience method for creating a binary classifier using the factory.
 */
std::unique_ptr<binary_classifier>
    make_binary_classifier(const cpptoml::toml_group& config,
                           std::shared_ptr<index::forward_index> idx,
                           class_label positive, class_label negative);

/**
 * Factory method for creating a binary classifier. This should be
 * specialized if your given binary classifier requires special
 * construction behavior (e.g., reading parameters).
 */
template <class Classifier>
std::unique_ptr<binary_classifier>
    make_binary_classifier(const cpptoml::toml_group&,
                           std::shared_ptr<index::forward_index> idx,
                           class_label positive, class_label negative)
{
    return make_unique<Classifier>(std::move(idx), positive, negative);
}

/**
 * Registration method for binary classifiers. Clients should use this
 * method to register any new binary classifiers they write.
 */
template <class Classifier>
void register_binary_classifier()
{
    binary_classifier_factory::get().add(Classifier::id,
                                         make_binary_classifier<Classifier>);
}
}
}
#endif
