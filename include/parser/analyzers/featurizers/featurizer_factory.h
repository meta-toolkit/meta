/**
 * @file featurizer_factory.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_ANALYZERS_TREE_FEATURIZER_FACTORY_H_
#define META_ANALYZERS_TREE_FEATURIZER_FACTORY_H_

#include "parser/analyzers/featurizers/tree_featurizer.h"
#include "util/factory.h"
#include "util/shim.h"

namespace cpptoml
{
class table;
}

namespace meta
{
namespace analyzers
{

/**
 * Factory that is responsible for creating tree featurizers from
 * configuration files.  Clients should use the register_featurizer method
 * instead of this class directly.
 */
template <class T>
class featurizer_factory
    : public util::factory<featurizer_factory<T>, tree_featurizer<T>>
{
    using base_factory = typename featurizer_factory::base_factory;
    using factory_method = typename featurizer_factory::factory_method;

    /// friend the base class
    friend base_factory;

  private:
    /**
     * Constructor.
     */
    featurizer_factory();

    /**
     * Adds (registers) a featurizer with this factory so it is able to be
     * created.
     */
    template <class Analyzer>
    void register_featurizer();

    /// maps id strings to the factory method used to create that class
    std::unordered_map<std::string, factory_method> methods_;
};

/**
 * Traits class for featurizers. You should specialize this class if you
 * need to customize creation behavior for your featurizer class. This is a
 * class template to allow for partial specializations as well.
 */
template <class Featurizer>
struct featurizer_traits
{
    static std::unique_ptr<typename Featurizer::base_type> create()
    {
        return make_unique<Featurizer>();
    }
};

/**
 * Factory method for creating a featurizer.
 */
template <class Featurizer>
std::unique_ptr<typename Featurizer::base_type> make_featurizer()
{
    return featurizer_traits<Featurizer>::create();
}

/**
 * Registration method for analyzers. Clients should use this method to
 * register any new filters they write.
 */
template <class Featurizer>
void register_featurizer()
{
    featurizer_factory<typename Featurizer::feature_value_type>::get().add(
        Featurizer::id, make_featurizer<Featurizer>);
}
}
}
#endif
