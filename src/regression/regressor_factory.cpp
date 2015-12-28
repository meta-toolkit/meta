/**
 * @file regressor_factory.cpp
 * @author Chase Geigle
 */

#include "meta/regression/models/sgd.h"
#include "meta/regression/regressor_factory.h"

namespace meta
{
namespace regression
{

template <class Regressor>
void regressor_factory::reg()
{
    add(Regressor::id, make_regressor<Regressor>);
}

regressor_factory::regressor_factory()
{
    // built-in regressors
    reg<sgd>();
}

std::unique_ptr<regressor> make_regressor(const cpptoml::table& config,
                                          regression_dataset_view training)
{
    auto id = config.get_as<std::string>("method");
    if (!id)
        throw regressor_factory::exception{
            "method required in regressor configuration"};

    return regressor_factory::get().create(*id, config, std::move(training));
}

template <class Regressor>
void regressor_loader::reg()
{
    add(Regressor::id, load_regressor<Regressor>);
}

regressor_loader::regressor_loader()
{
    // built-in regressors
    reg<sgd>();
}

std::unique_ptr<regressor> load_regressor(std::istream& in)
{
    std::string id;
    io::packed::read(in, id);
    return regressor_loader::get().create(id, in);
}
}
}
