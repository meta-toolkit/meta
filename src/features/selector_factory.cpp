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
}

std::unique_ptr<feature_selector>
    make_selector(const cpptoml::toml_group& config,
                  std::shared_ptr<index::forward_index> idx)
{
    auto group = config.get_group("features");
    if (!group)
        throw selector_factory::exception{
            "[features] group missing from config file"};

    auto prefix = group->get_as<std::string>("prefix");
    if (!prefix)
        throw selector_factory::exception{"no prefix in [features] group"};

    auto method = group->get_as<std::string>("method");
    if (!method)
        throw selector_factory::exception{
            "feature selection method required in [features] group"};

    uint64_t features_per_class = 20;
    auto num_features = group->get_as<int64_t>("features-per-class");
    if (num_features)
        features_per_class = *num_features;

    auto selector
        = selector_factory::get().create(*method, *group, std::move(idx));
    selector->init(features_per_class);
    return selector;
}
}
}
