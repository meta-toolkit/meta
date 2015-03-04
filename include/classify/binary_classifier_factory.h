/**
 * @file binary_classifier_factory.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_BINARY_CLASSIFIER_FACTORY_H_
#define META_BINARY_CLASSIFIER_FACTORY_H_

#include "classify/classifier/binary_classifier.h"
#include "util/factory.h"

namespace cpptoml
{
class table;
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
                           const cpptoml::table&,
                           std::shared_ptr<index::forward_index>, class_label,
                           class_label>
{
    friend base_factory;

  private:
    /**
     * Constructs the binary_classifier_factory singleton.
     */
    binary_classifier_factory();

    /**
     * Registers a classifier with the factory (used internally).
     */
    template <class Classifier>
    void reg();
};

// doxygen is being stupid here, so I'm forced to specify which overload
// I'm talking about in their brief...
/**
 * (Non-template): Convenience method for creating a binary classifier
 * using the factory.
 *
 * @param config The table that specifies the binary classifier's
 * configuration
 * @param idx The forward_index the binary classifier is being constructed
 * over
 * @param positive The class_label for positive documents
 * @param negative The class_label for negative documents
 *
 * @return a unique_ptr to a binary_classifier constructed from the given
 *  configuration
 */
std::unique_ptr<binary_classifier>
    make_binary_classifier(const cpptoml::table& config,
                           std::shared_ptr<index::forward_index> idx,
                           class_label positive, class_label negative);

/**
 * (Template): Factory method for creating a binary classifier; this should
 * be specialized if your given binary classifier requires special
 * construction behavior (e.g., reading parameters).
 *
 * @param config The table that specifies the binary classifier's
 * configuration
 * @param idx The forward_index the binary classifier is being constructed
 * over
 * @param positive The class_label for positive documents
 * @param negative The class_label for negative documents
 *
 * @return a unique_ptr to a binary_classifier (of derived type
 * Classifier) that has been constructed from the given configuration
 */
template <class Classifier>
std::unique_ptr<binary_classifier>
    make_binary_classifier(const cpptoml::table&,
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
