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

#include "meta/parser/analyzers/featurizers/tree_featurizer.h"
#include "meta/util/factory.h"
#include "meta/util/shim.h"

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
class featurizer_factory
    : public util::factory<featurizer_factory, tree_featurizer>
{
    using base_factory = featurizer_factory::base_factory;
    using factory_method = featurizer_factory::factory_method;

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
};

/**
 * Factory method for creating a featurizer. You should specialize this
 * method if you need to customize creation behavior for your featurizer
 * class.
 */
template <class Featurizer>
std::unique_ptr<tree_featurizer> make_featurizer()
{
    return make_unique<Featurizer>();
}

/**
 * Registration method for analyzers. Clients should use this method to
 * register any new filters they write.
 */
template <class Featurizer>
void register_featurizer()
{
    featurizer_factory::get().add(Featurizer::id, make_featurizer<Featurizer>);
}
}
}
#endif
