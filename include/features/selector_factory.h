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

#include "features/feature_selector.h"
#include "util/factory.h"
#include "util/shim.h"

namespace cpptoml
{
class table;
}

namespace meta
{
namespace features
{

/**
 * Factory that is responsible for creating selectors from configuration
 * files. Clients should use the register_selector method instead of this
 * class directly to add their own selectors.
 */
class selector_factory
    : public util::factory<selector_factory, feature_selector,
                           const cpptoml::table&,
                           std::shared_ptr<index::forward_index>>
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
 * @param idx The forward_index to be passed to the selector being
 * created
 *
 * @return a unique_ptr to the selector created from the given
 * configuration
 */
std::unique_ptr<feature_selector>
    make_selector(const cpptoml::table& config,
                  std::shared_ptr<index::forward_index> idx);

/**
 * Factory method for creating a ranker. This should be specialized if
 * your given ranker requires special construction behavior (e.g.,
 * reading parameters).
 */
template <class Selector>
std::unique_ptr<feature_selector>
    make_selector(const cpptoml::table& config,
                  std::shared_ptr<index::forward_index> idx)
{
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw selector_factory::exception{"no prefix in [features] table"};

    auto method = config.get_as<std::string>("method");
    if (!method)
        throw selector_factory::exception{
            "feature selection method required in [features] table"};

    return make_unique<Selector>(*prefix + "." + *method, std::move(idx));
}

/**
 * Registration method for selectors. Clients should use this method to
 * register any new selectors they write.
 */
template <class Selector>
void register_selector()
{
    selector_factory::get().add(Selector::id,
                                [](const cpptoml::table& config,
                                   std::shared_ptr<index::forward_index> idx)
                                {
        return make_selector<feature_selector>(config, std::move(idx));
    });
}
}
}
#endif
