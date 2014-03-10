/**
 * @file loss_function_factory.h
 * @author Chase Geigle
 */

#ifndef META_CLASSIFY_LOSS_FUNCTION_FACTORY_H_
#define META_CLASSIFY_LOSS_FUNCTION_FACTORY_H_

#include <functional>
#include <memory>
#include <unordered_map>

#include "classify/loss/loss_function.h"
#include "util/factory.h"
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
    : public util::factory<loss_function_factory, loss_function>
{
    friend base_factory;

  private:
    loss_function_factory();

    template <class Loss>
    void reg();
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
