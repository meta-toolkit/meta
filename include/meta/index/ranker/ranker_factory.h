/**
 * @file ranker_factory.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_RANKER_FACTORY_H_
#define META_RANKER_FACTORY_H_

#include "meta/index/ranker/lm_ranker.h"
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
    : public util::factory<ranker_factory, ranker, const cpptoml::table&,
                           const cpptoml::table&>
{
  public:
    /// Friend the base ranker factory
    friend base_factory;

    std::unique_ptr<language_model_ranker>
    create_lm(util::string_view identifier, const cpptoml::table& global,
              const cpptoml::table& local)
    {
        auto rnk = base_factory::create(identifier, global, local);
        if (auto der = dynamic_cast<language_model_ranker*>(rnk.get()))
        {
            rnk.release();
            return std::unique_ptr<language_model_ranker>{der};
        }
        throw std::invalid_argument{identifier.to_string()
                                    + " is not a language_model_ranker"};
    }

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
 * Convenience method for creating a ranker using the factory.
 * @param global The global configuration group (containing the index path)
 * @param local The ranker configuration group itself
 */
std::unique_ptr<ranker> make_ranker(const cpptoml::table& global,
                                    const cpptoml::table& local);

/**
 * Convenience method for creating a language_model_ranker using the
 * factory.
 */
std::unique_ptr<language_model_ranker> make_lm_ranker(const cpptoml::table&);

/**
 * Convenience method for creating a language_model_ranker using the factory.
 * @param global The global configuration group (containing the index path)
 * @param local The ranker configuration group itself
 */
std::unique_ptr<language_model_ranker>
make_lm_ranker(const cpptoml::table& global, const cpptoml::table& local);

/**
 * Factory method for creating a ranker. This should be specialized if
 * your given ranker requires special construction behavior (e.g.,
 * reading parameters) that requires only the ranker-specific configuration
 * (this will be the case almost all of the time).
 */
template <class Ranker>
std::unique_ptr<ranker> make_ranker(const cpptoml::table&)
{
    return make_unique<Ranker>();
}

/**
 * Factory method for creating a ranker. This should be specialized if your
 * given ranker requires special construction behavior that includes
 * reading parameter values from the global configuration as well as the
 * ranker-specific configuration.
 */
template <class Ranker>
std::unique_ptr<ranker> make_ranker(const cpptoml::table& global,
                                    const cpptoml::table& local)
{
    (void)global;
    return make_ranker<Ranker>(local);
}

/**
 * Factory that is responsible for loading rankers from streams. Clients
 * should use the register_ranker method instead of this class directly to
 * add their own rankers.
 */
class ranker_loader : public util::factory<ranker_loader, ranker, std::istream&>
{
  public:
    friend base_factory;

    std::unique_ptr<language_model_ranker>
    create_lm(util::string_view identifier, std::istream& in)
    {
        auto rnk = base_factory::create(identifier, in);
        if (auto lmr = dynamic_cast<language_model_ranker*>(rnk.get()))
        {
            rnk.release();
            return std::unique_ptr<language_model_ranker>{lmr};
        }
        throw std::invalid_argument{
            "loaded ranker is not a language_model_ranker"};
    }

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
 * Convenience method for loading a language_model_ranker using the factory.
 */
std::unique_ptr<language_model_ranker> load_lm_ranker(std::istream&);

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
    ranker_factory::get().add(Ranker::id, [](const cpptoml::table& global,
                                             const cpptoml::table& local) {
        return make_ranker<Ranker>(global, local);
    });
    ranker_loader::get().add(Ranker::id, load_ranker<Ranker>);
}
}
}
#endif
