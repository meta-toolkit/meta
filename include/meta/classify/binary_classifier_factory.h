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

#include "meta/classify/classifier/binary_classifier.h"
#include "meta/config.h"
#include "meta/util/factory.h"

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
                           const cpptoml::table&, binary_dataset_view>
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
 * @param training The training data
 *
 * @return a unique_ptr to a binary_classifier constructed from the given
 *  configuration, trained on the given training data
 */
std::unique_ptr<binary_classifier>
make_binary_classifier(const cpptoml::table& config,
                       binary_dataset_view training);

/**
 * (Template): Factory method for creating a binary classifier; this should
 * be specialized if your given binary classifier requires special
 * construction behavior (e.g., reading parameters).
 *
 * @param config The table that specifies the binary classifier's
 * configuration
 * @param training The training data
 *
 * @return a unique_ptr to a binary_classifier (of derived type
 * Classifier) that has been constructed from the given configuration
 */
template <class Classifier>
std::unique_ptr<binary_classifier>
make_binary_classifier(const cpptoml::table&, binary_dataset_view training)
{
    return make_unique<Classifier>(training);
}

/**
 * Factory that is responsible for loading binary_classifiers from input
 * streams.  Clients should use the register_binary_classifier method
 * instead of this class directly to add their own classifiers.
 */
class binary_classifier_loader
    : public util::factory<binary_classifier_loader, binary_classifier,
                           std::istream&>
{
    friend base_factory;

  private:
    /**
      * Constructs the binary_classifier_loader singleton.
      */
    binary_classifier_loader();

    /**
     * Registers a classifier. Used internally.
     */
    template <class Classifier>
    void reg();
};

/**
 * Convenience method for loading a binary_classifier using the factory.
 *
 * @param stream The stream to load the model from
 *
 * @return a unique_ptr to the classifier created from the given
 * stream
 */
std::unique_ptr<binary_classifier> load_binary_classifier(std::istream& stream);

/**
 * Factory method for loading a classifier. This should be specialized if
 * your given classifier requires special construction behavior (e.g.,
 * reading parameters).
 *
 * @param stream The stream to load the model from
 *
 * @return a unique_ptr to the classifier (of derived type Classifier)
 * created from the given stream
 */
template <class Classifier>
std::unique_ptr<binary_classifier> load_binary_classifier(std::istream& input)
{
    return make_unique<Classifier>(input);
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
    binary_classifier_loader::get().add(Classifier::id,
                                        load_binary_classifier<Classifier>);
}
}
}
#endif
