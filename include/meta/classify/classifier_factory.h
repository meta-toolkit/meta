/**
 * @file classifier_factory.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CLASSIFIER_FACTORY_H_
#define META_CLASSIFIER_FACTORY_H_

#include <istream>

#include "meta/classify/classifier/classifier.h"
#include "meta/config.h"
#include "meta/util/factory.h"
#include "meta/util/shim.h"

namespace cpptoml
{
class table;
}

namespace meta
{
namespace classify
{

/**
 * Factory that is responsible for creating classifiers from configuration
 * files. Clients should use the register_classifier method instead of this
 * class directly to add their own classifiers.
 */
class classifier_factory
    : public util::factory<classifier_factory, classifier,
                           const cpptoml::table&, multiclass_dataset_view,
                           std::shared_ptr<index::inverted_index>>
{
    friend base_factory;

  private:
    /**
     * Constructs the classifier_factory singleton.
     */
    classifier_factory();

    /**
     * Registers a single-index classifier. Used internally.
     */
    template <class Classifier>
    void reg();

    /**
     * Registers a multi-index classifier. Used internally.
     */
    template <class Classifier>
    void reg_mi();
};

/**
 * Convenience method for creating a classifier using the factory.
 *
 * @param config The configuration group that specifies the configuration
 * for the classifier to be created
 * @param inv_idx The inverted_index to be passed to the classifier being
 * created (if needed)
 *
 * @return a unique_ptr to the classifier created from the given
 * configuration
 */
std::unique_ptr<classifier>
make_classifier(const cpptoml::table& config, multiclass_dataset_view training,
                std::shared_ptr<index::inverted_index> inv_idx = nullptr);

/**
 * Factory method for creating a classifier. This should be specialized if
 * your given classifier requires special construction behavior (e.g.,
 * reading parameters).
 *
 * @param config The configuration group that specifies the configuration
 * for the classifier to be created
 *
 * @return a unique_ptr to the classifier (of derived type Classifier)
 * created from the given configuration
 */
template <class Classifier>
std::unique_ptr<classifier> make_classifier(const cpptoml::table&,
                                            multiclass_dataset_view training)
{
    return make_unique<Classifier>(training);
}

/**
 * Factory method for creating a classifier that takes both index types.
 * This should be specialized if your given classifier requires special
 * construction behavior.
 *
 * @param config The configuration group that specifies the configuration
 * for the classifier to be created
 * @param idx The forward_index to be passed to the classifier being
 * created
 * @param inv_idx The inverted_index to be passed to the classifier being
 * created
 *
 * @return a unique_ptr to the classifier (of derived type Classifier)
 * created from the given configuration
 */
template <class Classifier>
std::unique_ptr<classifier>
make_multi_index_classifier(const cpptoml::table&,
                            multiclass_dataset_view training,
                            std::shared_ptr<index::inverted_index> inv_idx)
{
    return make_unique<Classifier>(training, inv_idx);
}

/**
 * Factory that is responsible for loading classifiers from input streams.
 * Clients should use the register_classifier method instead of this class
 * directly to add their own classifiers.
 */
class classifier_loader
    : public util::factory<classifier_loader, classifier, std::istream&>
{
    friend base_factory;

  private:
    /**
      * Constructs the classifier_loader singleton.
      */
    classifier_loader();

    /**
     * Registers a classifier. Used internally.
     */
    template <class Classifier>
    void reg();
};

/**
 * Convenience method for loading a classifier using the factory.
 *
 * @param stream The stream to load the model from
 *
 * @return a unique_ptr to the classifier created from the given
 * stream
 */
std::unique_ptr<classifier> load_classifier(std::istream& stream);

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
std::unique_ptr<classifier> load_classifier(std::istream& input)
{
    return make_unique<Classifier>(input);
}

/**
 * Registration method for classifiers. Clients should use this method to
 * register any new classifiers they write that operate on just a
 * forward_index (this should be most).
 */
template <class Classifier>
void register_classifier()
{
    // wrap the make_classifier function to make it appear like it takes
    // two indexes, when we really only care about one.
    classifier_factory::get().add(
        Classifier::id,
        [](const cpptoml::table& config, multiclass_dataset_view training,
           std::shared_ptr<index::inverted_index>) {
            return make_classifier<Classifier>(config, training);
        });

    classifier_loader::get().add(Classifier::id, load_classifier<Classifier>);
}

/**
 * Registration method for multi-index classifiers. Clients should use this
 * method to register any new classifiers they write that operate on both a
 * forward_index and an inverted_index (this is rare).
 */
template <class Classifier>
void register_multi_index_classifier()
{
    classifier_factory::get().add(Classifier::id,
                                  make_multi_index_classifier<Classifier>);

    classifier_loader::get().add(Classifier::id, load_classifier<Classifier>);
}
}
}
#endif
