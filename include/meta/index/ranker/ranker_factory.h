/**
 * @file ranker_factory.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_RANKER_FACTORY_H_
#define META_RANKER_FACTORY_H_

#include "meta/index/ranker/ranker.h"
#include "meta/util/factory.h"
#include "meta/util/shim.h"

namespace cpptoml
{
class table;
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
    : public util::factory<ranker_factory, ranker, const cpptoml::table&>
{
    /// Friend the base ranker factory
    friend base_factory;

  private:
    /**
     * Constructor.
     */
    ranker_factory();

    /**
     * Registers a ranking function.
     */
    template <class Ranker>
    void reg();
};

/**
 * Convenience method for creating a ranker using the factory.
 */
std::unique_ptr<ranker> make_ranker(const cpptoml::table&);

/**
 * Factory method for creating a ranker. This should be specialized if
 * your given ranker requires special construction behavior (e.g.,
 * reading parameters).
 */
template <class Ranker>
std::unique_ptr<ranker> make_ranker(const cpptoml::table&)
{
    return make_unique<Ranker>();
}

/**
 * Factory that is responsible for loading rankers from streams. Clients
 * should use the register_ranker method instead of this class directly to
 * add their own rankers.
 */
class ranker_loader : public util::factory<ranker_loader, ranker, std::istream&>
{
    friend base_factory;

  private:
    /**
     * Constructor for setting up the singleton ranker_loader.
     */
    ranker_loader();

    /**
     * Registers a ranking function; used internally.
     */
    template <class Ranker>
    void reg();
};

/**
 * Convenience method for loading a ranker using the factory.
 */
std::unique_ptr<ranker> load_ranker(std::istream&);

/**
 * Factory method for loading a ranker. This should be specialized if your
 * given ranker requires special construction behavior. Otherwise, it is
 * assumed that the ranker has a constructor from a std::istream&.
 */
template <class Ranker>
std::unique_ptr<ranker> load_ranker(std::istream& in)
{
    return make_unique<Ranker>(in);
}

/**
 * Registration method for rankers. Clients should use this method to
 * register any new rankers they write.
 */
template <class Ranker>
void register_ranker()
{
    ranker_factory::get().add(Ranker::id, make_ranker<Ranker>);
    ranker_loader::get().add(Ranker::id, load_ranker<Ranker>);
}
}
}
#endif
