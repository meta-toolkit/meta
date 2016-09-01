/**
 * @file regressor_factory.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_REGRESSION_REGRESSION_FACTORY_H_
#define META_REGRESSION_REGRESSION_FACTORY_H_

#include <istream>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/regression/models/regressor.h"
#include "meta/util/factory.h"
#include "meta/util/shim.h"

namespace meta
{
namespace regression
{

/**
 * Factory that is responsible for creating regressors from configuration
 * files. Clients should use the register_regressor method instead of this
 * class directly to add their own regression models.
 */
class regressor_factory
    : public util::factory<regressor_factory, regressor, const cpptoml::table&,
                           regression_dataset_view>
{
    friend base_factory;

    /**
     * Constructs the regressor_factory singleton.
     */
    regressor_factory();

    /**
     * Registers a regressor. Used internally.
     */
    template <class Regressor>
    void reg();
};

/**
 * Convenience method for creating a regressor using the factory.
 *
 * @param config The configuration group that specifies the configuration
 * for the regressor to be created.
 * @param training The training set to learn the regression model over
 *
 * @return a unique_ptr to a regressor created from the configuration and
 * training data
 */
std::unique_ptr<regressor> make_regressor(const cpptoml::table& config,
                                          regression_dataset_view training);

/**
 * Factory method for creating a regressor. This should be specialized if
 * your given regressor requires special construction behavior (e.g.,
 * reading parameters).
 *
 * @param config The configuration group that specifies the configuration
 * for the regressor to be created.
 * @param training The training set to learn the regression model over
 *
 * @return a unique_ptr to the regressor (of derived type Regressor)
 * created from the given configuration
 */
template <class Regressor>
std::unique_ptr<regressor> make_regressor(const cpptoml::table&,
                                          regression_dataset_view training)
{
    return make_unique<Regressor>(training);
}

/**
 * Factory that is responsible for loading regressors from input streams.
 * Clients should use the register_regressor method instead of this class
 * directly to add their own classifiers.
 */
class regressor_loader
    : public util::factory<regressor_loader, regressor, std::istream&>
{
    friend base_factory;

    /**
      * Constructs the regressor_loader singleton.
      */
    regressor_loader();

    /**
     * Registers a regressor. Used internally.
     */
    template <class Regressor>
    void reg();
};

/**
 * Convenience method for loading a regressor using the factory.
 *
 * @param stream The stream to load the model from
 *
 * @return a unique_ptr to the regressor created from the given
 * stream
 */
std::unique_ptr<regressor> load_regressor(std::istream& stream);

/**
 * Factory method for loading a regressor. This should be specialized if
 * your given regressor requires special construction behavior (e.g.,
 * reading parameters).
 *
 * @param stream The stream to load the model from
 *
 * @return a unique_ptr to the regressor (of derived type Regressor)
 * created from the given stream
 */
template <class Regressor>
std::unique_ptr<regressor> load_regressor(std::istream& input)
{
    return make_unique<Regressor>(input);
}

/**
 * Registration method for regressors. Clients should use this method to
 * register any new regressors they write.
 */
template <class Regressor>
void register_regressor()
{
    regressor_factory::get().add(Regressor::id, make_regressor<Regressor>);
    regressor_loader::get().add(Regressor::id, load_regressor<Regressor>);
}
}
}
#endif
