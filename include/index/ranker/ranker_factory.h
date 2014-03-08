/**
 * @file ranker_factory.h
 * @author Chase Geigle
 */

#ifndef _META_RANKER_FACTORY_H_
#define _META_RANKER_FACTORY_H_

#include "index/ranker/ranker.h"
#include "util/factory.h"
#include "util/shim.h"

namespace cpptoml
{
class toml_group;
}

namespace meta
{
namespace index
{

/**
 * Factory that is responsible for creating rankers from configuration
 * files. Clients should use the register_ranker method instead of this
 * class directly to add their own rankers.
 */
class ranker_factory
    : public util::factory<ranker_factory, ranker, const cpptoml::toml_group&>
{
    friend base_factory;

  private:
    ranker_factory();

    template <class Ranker>
    void reg();
};

/**
 * Convenience method for creating a ranker using the factory.
 */
std::unique_ptr<ranker> make_ranker(const cpptoml::toml_group&);

/**
 * Factory method for creating a ranker. This should be specialized if
 * your given ranker requires special construction behavior (e.g.,
 * reading parameters).
 */
template <class Ranker>
std::unique_ptr<ranker> make_ranker(const cpptoml::toml_group&)
{
    return make_unique<Ranker>();
}
}
}

#endif
