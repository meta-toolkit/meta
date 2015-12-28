/**
 * @file loss_function_factory.cpp
 * @author Chase Geigle
 */

#include "meta/learn/loss/all.h"
#include "meta/learn/loss/loss_function_factory.h"
#include "meta/io/packed.h"

namespace meta
{
namespace learn
{
namespace loss
{

template <class Loss>
void loss_function_factory::reg()
{
    add(Loss::id, make_loss_function<Loss>);
}

loss_function_factory::loss_function_factory()
{
    // built-in loss functions
    reg<hinge>();
    reg<huber>();
    reg<least_squares>();
    reg<logistic>();
    reg<modified_huber>();
    reg<perceptron>();
    reg<smooth_hinge>();
    reg<squared_hinge>();
}

std::unique_ptr<loss_function> make_loss_function(const std::string& identifier)
{
    return loss_function_factory::get().create(identifier);
}

std::unique_ptr<loss_function> load_loss_function(std::istream& in)
{
    auto id = io::packed::read<std::string>(in);
    return loss_function_factory::get().create(id);
}
}
}
}
