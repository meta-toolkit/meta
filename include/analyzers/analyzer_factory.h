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

#include "analyzers/analyzer.h"
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
 * Factory that is responsible for creating analyzers from configuration
 * files.  Clients should use the register_analyzer method instead of this
 * class directly.
 */
template <class T>
class analyzer_factory
    : public util::factory<analyzer_factory<T>, analyzer<T>,
                           const cpptoml::table&, const cpptoml::table&>
{
    using base_factory = typename analyzer_factory::base_factory;
    using factory_method = typename base_factory::factory_method;

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

extern template class analyzer_factory<uint64_t>;
extern template class analyzer_factory<double>;

/**
 * Traits class for analyzers. You should specialize this class if you need
 * to customize creation behavior for your analyzer class. This is a class
 * template to allow for partial specializations as well.
 */
template <class Analyzer>
struct analyzer_traits
{
    static std::unique_ptr<typename Analyzer::base_type>
        create(const cpptoml::table&, const cpptoml::table&)
    {
        return make_unique<Analyzer>();
    }
};

/**
 * Factory method for creating an analyzer.
 */
template <class Analyzer>
std::unique_ptr<typename Analyzer::base_type>
    make_analyzer(const cpptoml::table& global, const cpptoml::table& config)
{
    return analyzer_traits<Analyzer>::create(global, config);
}

/**
 * Registration method for analyzers. Clients should use this method to
 * register any new filters they write.
 */
template <class Analyzer>
void register_analyzer()
{
    analyzer_factory<typename Analyzer::feature_value_type>::get().add(
        Analyzer::id, make_analyzer<Analyzer>);
}
}
}
#endif
