/**
 * @file analyzer_factory.h
 * @author Chase Geigle
 */

#ifndef _META_ANALYZER_FACTORY_H_
#define _META_ANALYZER_FACTORY_H_

#include <functional>
#include <memory>
#include <unordered_map>

#include "analyzers/analyzer.h"
#include "util/shim.h"

namespace cpptoml
{
class toml_group;
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
{
  public:
    using pointer = std::unique_ptr<analyzer>;
    using factory_method = std::function<
        pointer(const cpptoml::toml_group&, const cpptoml::toml_group&)>;
    using exception = analyzer::analyzer_exception;

    /**
     * Obtains the singleton.
     */
    inline static analyzer_factory& get()
    {
        static analyzer_factory factory;
        return factory;
    }

    /**
     * Associates a given identifier with the given factory method.
     */
    template <class Function>
    void add(const std::string& identifier, Function&& fn)
    {
        if (methods_.find(identifier) != methods_.end())
            throw exception{"analyzer already registered with that id"};
        methods_.emplace(identifier, std::forward<Function>(fn));
    }

    /**
     * Creates a new analyzer based on the identifier and configuration
     * group.
     */
    pointer create(const std::string& identifier,
                   const cpptoml::toml_group& global,
                   const cpptoml::toml_group& config);

  private:
    analyzer_factory();

    template <class Analyzer>
    void register_analyzer();

    std::unordered_map<std::string, factory_method> methods_;
};

/**
 * Factory method for creating an analyzer. This should be specialized if
 * your given tokenizer requires special construction behavior.
 */
template <class Analyzer>
std::unique_ptr<analyzer> make_analyzer(const cpptoml::toml_group&,
                                        const cpptoml::toml_group&)
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
