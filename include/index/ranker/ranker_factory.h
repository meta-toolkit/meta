/**
 * @file ranker_factory.h
 * @author Chase Geigle
 */

#ifndef _META_RANKER_FACTORY_H_
#define _META_RANKER_FACTORY_H_

#include <memory>
#include <unordered_map>
#include "index/ranker/ranker.h"
#include "util/shim.h"

namespace cpptoml
{
class toml_group;
}

namespace meta
{
namespace index
{

/**
 * Factory that is responsible for creating rankers from configuration
 * files. Clients should use the register_ranker method instead of this
 * class directly to add their own rankers.
 */
class ranker_factory
{
  public:
    using pointer = std::unique_ptr<ranker>;
    using factory_method = std::function<pointer(const cpptoml::toml_group&)>;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    /**
     * Obtains the singleton.
     */
    inline static ranker_factory& get()
    {
        static ranker_factory factory;
        return factory;
    }

    /**
     * Associates the given identifier with the given factory method.
     */
    template <class Function>
    void add(const std::string& identifier, Function&& fn)
    {
        if (methods_.find(identifier) != methods_.end())
            throw exception{"classifier already registered with that id"};
        methods_.emplace(identifier, std::forward<Function>(fn));
    }

    /**
     * Creates a new classifier based on the identifier, configuration
     * object, and index(es).
     */
    pointer create(const std::string& identifier,
                   const cpptoml::toml_group& config);

  private:
    ranker_factory();

    template <class Ranker>
    void add();

    std::unordered_map<std::string, factory_method> methods_;
};

/**
 * Convenience method for creating a ranker using the factory.
 */
std::unique_ptr<ranker> make_ranker(const cpptoml::toml_group&);

/**
 * Factory method for creating a ranker. This should be specialized if
 * your given ranker requires special construction behavior (e.g.,
 * reading parameters).
 */
template <class Ranker>
std::unique_ptr<ranker> make_ranker(const cpptoml::toml_group&)
{
    return make_unique<Ranker>();
}

}
}

#endif
