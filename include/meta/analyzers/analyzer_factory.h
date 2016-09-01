/**
 * @file analyzer_factory.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_ANALYZER_FACTORY_H_
#define META_ANALYZER_FACTORY_H_

#include "meta/analyzers/analyzer.h"
#include "meta/config.h"
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
 * Factory that is responsible for creating analyzers from configuration
 * files.  Clients should use the register_analyzer method instead of this
 * class directly.
 */
class analyzer_factory
    : public util::factory<analyzer_factory, analyzer, const cpptoml::table&,
                           const cpptoml::table&>
{
    using base_factory = analyzer_factory::base_factory;
    using factory_method = base_factory::factory_method;

    /// friend the base class
    friend base_factory;

  private:
    /**
     * Constructor.
     */
    analyzer_factory();

    /**
     * Adds (registers) an analyzer with this factory so it is able to be
     * created.
     */
    template <class Analyzer>
    void register_analyzer();

    /// maps id strings to the factory method used to create that class
    std::unordered_map<std::string, factory_method> methods_;
};

/**
 * Factory method for creating an analyzer. You should specialize this
 * method if you need to customize creation behavior for your analyzer
 * class.
 */
template <class Analyzer>
std::unique_ptr<analyzer> make_analyzer(const cpptoml::table&,
                                        const cpptoml::table&)
{
    return make_unique<Analyzer>();
}

/**
 * Registration method for analyzers. Clients should use this method to
 * register any new filters they write.
 */
template <class Analyzer>
void register_analyzer()
{
    analyzer_factory::get().add(Analyzer::id, make_analyzer<Analyzer>);
}
}
}
#endif
