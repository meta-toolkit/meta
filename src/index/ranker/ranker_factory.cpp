/**
 * @file ranker_factory.cpp
 * @author Chase Geigle
 */

#include "cpptoml.h"
#include "meta/index/ranker/all.h"
#include "meta/index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

template <class Ranker>
void ranker_factory::reg()
{
    add(Ranker::id, make_ranker<Ranker>);
}

ranker_factory::ranker_factory()
{
    // built-in rankers
    reg<absolute_discount>();
    reg<dirichlet_prior>();
    reg<jelinek_mercer>();
    reg<okapi_bm25>();
    reg<pivoted_length>();
}

std::unique_ptr<ranker> make_ranker(const cpptoml::table& config)
{
    auto function = config.get_as<std::string>("method");
    if (!function)
        throw ranker_factory::exception{
            "ranking-function required to construct a ranker"};
    return ranker_factory::get().create(*function, config);
}

template <class Ranker>
void ranker_loader::reg()
{
    add(Ranker::id, load_ranker<Ranker>);
}

ranker_loader::ranker_loader()
{
    // built-in rankers
    reg<absolute_discount>();
    reg<dirichlet_prior>();
    reg<jelinek_mercer>();
    reg<okapi_bm25>();
    reg<pivoted_length>();
}

std::unique_ptr<ranker> load_ranker(std::istream& in)
{
    std::string method;
    io::packed::read(in, method);
    return ranker_loader::get().create(method, in);
}
}
}
