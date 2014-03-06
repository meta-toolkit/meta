/**
 * @file loss_function_factory.cpp
 * @author Chase Geigle
 */

#include "classify/loss/all.h"
#include "classify/loss/loss_function_factory.h"

namespace meta
{
namespace classify
{
namespace loss
{

template <class Loss>
void loss_function_factory::add()
{
    add(Loss::id, make_loss_function<Loss>);
}

loss_function_factory::loss_function_factory()
{
    // built-in loss functions
    add<hinge>();
    add<huber>();
    add<least_squares>();
    add<logistic>();
    add<modified_huber>();
    add<perceptron>();
    add<smooth_hinge>();
    add<squared_hinge>();
}

auto loss_function_factory::create(const std::string& identifier) -> pointer
{
    if (methods_.find(identifier) == methods_.end())
        throw exception{"unrecognized classifier id"};
    return methods_[identifier]();
}

std::unique_ptr<loss_function>
    make_loss_function(const std::string& identifier)
{
    return loss_function_factory::get().create(identifier);
}

}
}
}
