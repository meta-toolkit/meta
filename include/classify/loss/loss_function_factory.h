/**
 * @file loss_function_factory.h
 * @author Chase Geigle
 */

#ifndef _META_CLASSIFY_LOSS_FUNCTION_FACTORY_H_
#define _META_CLASSIFY_LOSS_FUNCTION_FACTORY_H_

#include <functional>
#include <memory>
#include <unordered_map>

#include "classify/loss/loss_function.h"
#include "util/shim.h"

namespace meta
{
namespace classify
{
namespace loss
{

/**
 * Factory that is responsible for creating loss functions from strings.
 * Clients should use the register_loss_function method instead of this
 * class directly to add their own loss functions.
 */
class loss_function_factory
{
  public:
    using pointer = std::unique_ptr<loss_function>;
    using factory_method = std::function<pointer()>;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    /**
     * Obtains the singleton.
     */
    inline static loss_function_factory& get()
    {
        static loss_function_factory factory;
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
     * Creates a new loss function based on the identifier.
     */
    pointer create(const std::string& identifier);

  private:
    loss_function_factory();

    template <class Loss>
    void add();

    std::unordered_map<std::string, factory_method> methods_;
};

/**
 * Convenience method for making a loss function using the factory.
 */
std::unique_ptr<loss_function>
    make_loss_function(const std::string& identifier);

/**
 * Factory method for creating a loss function.
 */
template <class Loss>
std::unique_ptr<loss_function> make_loss_function()
{
    return make_unique<Loss>();
}

/**
 * Registration method for loss functions. Clients should use this method
 * to register any new loss functions they write.
 */
template <class Loss>
void register_loss_function()
{
    loss_function_factory::get().add(Loss::id, make_loss_function<Loss>);
}

}
}
}

#endif
