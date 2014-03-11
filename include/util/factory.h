/**
 * @file factory.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_UTIL_FACTORY_H_
#define META_UTIL_FACTORY_H_

#include <functional>
#include <memory>
#include <unordered_map>

namespace meta
{
namespace util
{

/**
 * Generic factory that can be subclassed to create factories for specific
 * types.
 */
template <class DerivedFactory, class Type, class... Arguments>
class factory
{
  public:
    using base_factory = factory;
    using pointer = std::unique_ptr<Type>;
    using factory_method = std::function<pointer(Arguments...)>;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    /**
     * Obtains the singleton.
     */
    inline static DerivedFactory& get()
    {
        static DerivedFactory factory;
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
     * Creates a new object based on the factory method parameters.
     */
    template <class... Args>
    pointer create(const std::string& identifier, Args&&... args)
    {
        if (methods_.find(identifier) == methods_.end())
            throw exception{"unrecognized identifier"};
        return methods_[identifier](std::forward<Args>(args)...);
    }

  private:
    std::unordered_map<std::string, factory_method> methods_;
};
}
}
#endif
