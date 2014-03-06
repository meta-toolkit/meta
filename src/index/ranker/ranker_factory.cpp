/**
 * @file ranker_factory.cpp
 * @author Chase Geigle
 */

#include "cpptoml.h"
#include "index/ranker/all.h"
#include "index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

template <class Ranker>
void ranker_factory::add()
{
    add(Ranker::id, make_ranker<Ranker>);
}

ranker_factory::ranker_factory()
{
    // built-in rankers
    add<absolute_discount>();
    add<dirichlet_prior>();
    add<jelinek_mercer>();
    add<okapi_bm25>();
    add<pivoted_length>();
}

auto ranker_factory::create(const std::string& identifier,
                            const cpptoml::toml_group& config) -> pointer
{

    if (methods_.find(identifier) == methods_.end())
        throw exception{"unrecognized ranker id"};
    return methods_[identifier](config);
}

std::unique_ptr<ranker> make_ranker(const cpptoml::toml_group& config)
{
    auto function = config.get_as<std::string>("method");
    if (!function)
        throw ranker_factory::exception{
            "ranking-function required to construct a ranker"};
    return ranker_factory::get().create(*function, config);
}

}
}
