/**
 * @file selector_factory.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "features/all.h"
#include "features/selector_factory.h"

namespace meta
{
namespace features
{

template <class Selector>
void selector_factory::reg()
{
    add(Selector::id, make_selector<Selector>);
}

selector_factory::selector_factory()
{
    // built-in feature-selection algorithms
    reg<information_gain>();
    reg<chi_square>();
    reg<correlation_coefficient>();
    reg<odds_ratio>();
}

std::unique_ptr<feature_selector>
    make_selector(const cpptoml::table& config,
                  std::shared_ptr<index::forward_index> idx)
{
    auto table = config.get_table("features");
    if (!table)
        throw selector_factory_exception{
            "[features] table missing from config file"};

    auto prefix = table->get_as<std::string>("prefix");
    if (!prefix)
        throw selector_factory_exception{"no prefix in [features] table"};

    auto method = table->get_as<std::string>("method");
    if (!method)
        throw selector_factory_exception{
            "feature selection method required in [features] table"};

    uint64_t features_per_class
        = table->get_as<int64_t>("features-per-class").value_or(20);

    auto selector
        = selector_factory::get().create(*method, *table, std::move(idx));
    selector->init(features_per_class); // make_selector is a friend
    return selector;
}
}
}
