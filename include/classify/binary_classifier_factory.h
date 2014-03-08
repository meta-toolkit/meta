/**
 * @file binary_classifier_factory.h
 * @author Chase Geigle
 */

#ifndef _META_BINARY_CLASSIFIER_FACTORY_H_
#define _META_BINARY_CLASSIFIER_FACTORY_H_

#include "classify/classifier_factory.h"
#include "classify/classifier/binary_classifier.h"

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
{
  public:
    using pointer = std::unique_ptr<binary_classifier>;
    using factory_method = std::function<pointer(
        const cpptoml::toml_group&, std::shared_ptr<index::forward_index>,
        class_label, class_label)>;
    using exception = classifier_factory::exception;

    /**
     * Obtains the singleton.
     */
    inline static binary_classifier_factory& get()
    {
        static binary_classifier_factory factory;
        return factory;
    }

    /**
     * Associates the given identifier with the given factory method.
     */
    template <class Function>
    void add(const std::string& identifier, Function&& fn)
    {
        if (methods_.find(identifier) != methods_.end())
            throw exception{"binary classifier already registered with that id"};
        methods_.emplace(identifier, std::forward<Function>(fn));
    }

    /**
     * Creates a new binary classifier based on the identifier,
     * configuration object, index, positive label, and negative label.
     */
    pointer create(const std::string& identifier,
                   const cpptoml::toml_group& config,
                   std::shared_ptr<index::forward_index> idx,
                   class_label positive, class_label negative);
  private:
    binary_classifier_factory();

    template <class Classifier>
    void add();

    std::unordered_map<std::string, factory_method> methods_;
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
