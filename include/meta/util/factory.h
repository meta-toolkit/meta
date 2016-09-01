/**
 * @file factory.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_FACTORY_H_
#define META_UTIL_FACTORY_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "meta/config.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace util
{

/** Simple exception for factories */
class factory_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * Generic factory that can be subclassed to create factories for specific
 * types.
 */
template <class DerivedFactory, class Type, class... Arguments>
class factory
{
  public:
    /// Convenience typedef for the derived classes
    using base_factory = factory;
    /// The return type for the create method
    using pointer = std::unique_ptr<Type>;
    /// Convenience typedef for the factory methods used to create objects
    using factory_method = std::function<pointer(Arguments...)>;
    using exception = factory_exception;

    /**
     * Obtains the singleton.
     * @return a singleton of type DerivedFactory
     */
    inline static DerivedFactory& get()
    {
        static DerivedFactory factory;
        return factory;
    }

    /**
     * Associates the given identifier with the given factory method.
     * @param identifier The identifier to associate the factory method
     * with
     * @param fn The factory method
     */
    template <class Function>
    void add(util::string_view identifier, Function&& fn)
    {
        if (methods_.find(identifier) != methods_.end())
            throw exception{"classifier already registered with that id"};
        methods_.emplace(identifier, std::forward<Function>(fn));
    }

    /**
     * Creates a new object based on the factory method parameters.
     *
     * @param identifier The identifier to use to select a factory method
     * @param args The arguments to forward to that factory method
     * @return a unique_ptr to the new object created
     */
    template <class... Args>
    pointer create(util::string_view identifier, Args&&... args)
    {
        if (methods_.find(identifier) == methods_.end())
            throw exception{"unrecognized identifier: \""
                            + identifier.to_string() + "\""};
        return methods_[identifier](std::forward<Args>(args)...);
    }

  private:
    /// The internal map of identifiers to factory_methods.
    std::unordered_map<util::string_view, factory_method> methods_;
};
}
}
#endif
