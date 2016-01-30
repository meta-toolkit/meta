/**
 * @file loss_function_factory.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_LOSS_FUNCTION_FACTORY_H_
#define META_CLASSIFY_LOSS_FUNCTION_FACTORY_H_

#include <istream>
#include <functional>
#include <memory>
#include <unordered_map>

#include "meta/learn/loss/loss_function.h"
#include "meta/util/factory.h"
#include "meta/util/shim.h"

namespace meta
{
namespace learn
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
    /**
     * Constructs the loss_function_factory singleton.
     */
    loss_function_factory();

    /**
     * Registers a loss function. Used internally.
     */
    template <class Loss>
    void reg();
};

/**
 * Convenience method for making a loss function using the factory.
 * @param identifier the identifier for the loss function to be created
 * @return a unique_ptr to the loss function created
 */
std::unique_ptr<loss_function>
    make_loss_function(const std::string& identifier);

/**
 * Factory method for creating a loss function.
 * @return a unique_ptr to a loss_function (of derived type Loss)
 */
template <class Loss>
std::unique_ptr<loss_function> make_loss_function()
{
    return make_unique<Loss>();
}

/**
 * Convenience method for loading a loss function from a stream.
 */
std::unique_ptr<loss_function> load_loss_function(std::istream&);

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
