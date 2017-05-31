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
    add(Ranker::id,
        [](const cpptoml::table& global, const cpptoml::table& local) {
            return make_ranker<Ranker>(global, local);
        });
}

ranker_factory::ranker_factory()
{
    // built-in rankers
    reg<absolute_discount>();
    reg<dirichlet_prior>();
    reg<jelinek_mercer>();
    reg<okapi_bm25>();
    reg<pivoted_length>();
    reg<kl_divergence_prf>();
    reg<rocchio>();
}

std::unique_ptr<ranker> make_ranker(const cpptoml::table& config)
{
    // pass a blank configuration group as the first argument to the
    // factory method
    static auto blank = cpptoml::make_table();
    return make_ranker(*blank, config);
}

std::unique_ptr<ranker> make_ranker(const cpptoml::table& global,
                                    const cpptoml::table& local)
{
    auto function = local.get_as<std::string>("method");
    if (!function)
        throw ranker_factory::exception{
            "method key required in [ranker] to construct a ranker"};

    return ranker_factory::get().create(*function, global, local);
}

std::unique_ptr<language_model_ranker>
make_lm_ranker(const cpptoml::table& config)
{
    // pass a blank configuration group as the first argument to the
    // factory method
    static auto blank = cpptoml::make_table();
    return make_lm_ranker(*blank, config);
}

std::unique_ptr<language_model_ranker>
make_lm_ranker(const cpptoml::table& global, const cpptoml::table& local)
{
    auto function = local.get_as<std::string>("method");
    if (!function)
        throw ranker_factory::exception{
            "method key required in [ranker] to construct a ranker"};

    return ranker_factory::get().create_lm(*function, global, local);
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
    reg<kl_divergence_prf>();
    reg<rocchio>();
}

std::unique_ptr<ranker> load_ranker(std::istream& in)
{
    std::string method;
    io::packed::read(in, method);
    return ranker_loader::get().create(method, in);
}

std::unique_ptr<language_model_ranker> load_lm_ranker(std::istream& in)
{
    std::string method;
    io::packed::read(in, method);
    return ranker_loader::get().create_lm(method, in);
}
}
}
