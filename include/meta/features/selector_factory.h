/**
 * @file selector_factory.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FEATURE_SELECTOR_FACTORY_H_
#define META_FEATURE_SELECTOR_FACTORY_H_

#include "meta/classify/binary_dataset_view.h"
#include "meta/classify/multiclass_dataset.h"
#include "meta/classify/multiclass_dataset_view.h"
#include "meta/config.h"
#include "meta/features/feature_selector.h"
#include "meta/regression/regression_dataset_view.h"
#include "meta/util/factory.h"
#include "meta/util/shim.h"

namespace cpptoml
{
class table;
}

namespace meta
{
namespace features
{

/**
 * Exception for selector_factory operations.
 */
class selector_factory_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * Factory that is responsible for creating selectors from configuration
 * files. Clients should use the register_selector method instead of this
 * class directly to add their own selectors.
 */
class selector_factory
    : public util::factory<selector_factory, feature_selector,
                           const cpptoml::table&, uint64_t, uint64_t>
{
    friend base_factory;

  private:
    /**
     * Constructs the selector_factory singleton.
     */
    selector_factory();

    /**
     * Registers a single-index selector. Used internally.
     */
    template <class Selector>
    void reg();
};

/**
 * Convenience method for creating a selector using the factory.
 *
 * @param config The configuration table that specifies the configuration
 * for the selector to be created
 * @param docs The labeled dataset or dataset view (i.e., binary or multiclass
 * or regression)
 * to be passed to the selector being created
 *
 * @return a unique_ptr to the selector created from the given
 * configuration
 */
template <class LabeledDatasetContainer>
std::unique_ptr<feature_selector>
make_selector(const cpptoml::table& config, const LabeledDatasetContainer& docs)
{
    static_assert(
        std::is_same<classify::binary_dataset, LabeledDatasetContainer>::value
            || std::is_same<classify::binary_dataset_view,
                            LabeledDatasetContainer>::value
            || std::is_same<classify::multiclass_dataset,
                            LabeledDatasetContainer>::value
            || std::is_same<classify::multiclass_dataset_view,
                            LabeledDatasetContainer>::value,
        "docs should be a binary/multiclass dataset or dataset "
        "view");

    auto table = config.get_table("features");
    if (!table)
        throw selector_factory_exception{
            "[features] table missing from config file"};

    auto prefix = table->get_as<std::string>("prefix");
    if (!prefix)
        throw selector_factory_exception{"no prefix in [features] table"};

    auto method = table->get_as<std::string>("method");
    if (!method)
        throw selector_factory_exception{
            "feature selection method required in [features] table"};

    auto features_per_class = static_cast<uint64_t>(
        table->get_as<int64_t>("features-per-class").value_or(20));

    auto selector = selector_factory::get().create(
        *method, *table, docs.total_labels(), docs.total_features());

    selector->init(docs, features_per_class); // make_selector is a friend

    return selector;
}

/**
 * Factory method for creating a feature selector. This should be specialized if
 * your selector requires special construction behavior (e.g., reading
 * parameters).
 */
template <class Selector>
std::unique_ptr<feature_selector>
factory_make_selector(const cpptoml::table& config, uint64_t total_labels,
                      uint64_t total_features)
{
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw selector_factory_exception{"no prefix in [features] table"};

    auto method = config.get_as<std::string>("method");
    if (!method)
        throw selector_factory_exception{
            "feature selection method required in [features] table"};

    return make_unique<Selector>(*prefix + "/" + *method, total_labels,
                                 total_features);
}

/**
 * Registration method for selectors. Clients should use this method to
 * register any new selectors they write.
 */
template <class Selector>
void register_selector()
{
    selector_factory::get().add(Selector::id, factory_make_selector<Selector>);
}
}
}
#endif
