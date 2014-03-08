/**
 * @file classifier_factory.h
 * @author Chase Geigle
 */

#ifndef _META_CLASSIFIER_FACTORY_H_
#define _META_CLASSIFIER_FACTORY_H_

#include "classify/classifier/classifier.h"
#include "util/shim.h"

namespace cpptoml
{
class toml_group;
}

namespace meta
{
namespace classify
{

/**
 * Factory that is responsible for creating classifiers from configuration
 * files. Clients should use the register_classifier method instead of this
 * class directly to add their own classifiers.
 */
class classifier_factory
{
  public:
    using pointer = std::unique_ptr<classifier>;
    using factory_method = std::function<pointer(
        const cpptoml::toml_group&, std::shared_ptr<index::forward_index>,
        std::shared_ptr<index::inverted_index>)>;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    /**
     * Obtains the singleton.
     */
    inline static classifier_factory& get()
    {
        static classifier_factory factory;
        return factory;
    }

    /**
     * Associates the given identifier with the given factory method.
     */
    template <class Function>
    void add(const std::string& identifier, Function&& fn)
    {
        if (methods_.find(identifier) != methods_.end())
            throw exception{"classifier already registered with that id"};
        methods_.emplace(identifier, std::forward<Function>(fn));
    }

    /**
     * Creates a new classifier based on the identifier, configuration
     * object, and index(es).
     */
    pointer create(const std::string& identifier,
                   const cpptoml::toml_group& config,
                   std::shared_ptr<index::forward_index> idx,
                   std::shared_ptr<index::inverted_index> inv_idx = nullptr);

  private:
    classifier_factory();

    template <class Classifier>
    void add();

    template <class Classifier>
    void add_mi();

    std::unordered_map<std::string, factory_method> methods_;
};

/**
 * Convenience method for creating a classifier using the factory.
 */
std::unique_ptr<classifier>
    make_classifier(const cpptoml::toml_group& config,
                    std::shared_ptr<index::forward_index> idx,
                    std::shared_ptr<index::inverted_index> inv_idx = nullptr);

/**
 * Factory method for creating a classifier. This should be specialized if
 * your given classifier requires special construction behavior (e.g.,
 * reading parameters).
 */
template <class Classifier>
std::unique_ptr<classifier>
    make_classifier(const cpptoml::toml_group&,
                    std::shared_ptr<index::forward_index> idx)
{
    return make_unique<Classifier>(idx);
}

/**
 * Factory method for creating a classifier that takes both index types.
 * This should be specialized if your given classifier requires special
 * construction behavior.
 */
template <class Classifier>
std::unique_ptr<classifier>
    make_multi_index_classifier(const cpptoml::toml_group&,
                                std::shared_ptr<index::forward_index> idx,
                                std::shared_ptr<index::inverted_index> inv_idx)
{
    return make_unique<Classifier>(idx, inv_idx);
}

/**
 * Registration method for classifiers. Clients should use this method to
 * register any new classifiers they write that operate on just a
 * forward_index (this should be most).
 */
template <class Classifier>
void register_classifier()
{
    // wrap the make_classifier function to make it appear like it takes
    // two indexes, when we really only care about one.
    classifier_factory::get().add(Classifier::id,
                                  [](const cpptoml::toml_group& config,
                                     std::shared_ptr<index::forward_index> idx,
                                     std::shared_ptr<index::inverted_index>)
    {
        return make_classifier<Classifier>(config, std::move(idx));
    });
}

/**
 * Registration method for multi-index classifiers. Clients should use this
 * method to register any new classifiers they write that operate on both a
 * forward_index and an inverted_index (this is rare).
 */
template <class Classifier>
void register_multi_index_classifier()
{
    classifier_factory::get().add(Classifier::id,
                                  make_multi_index_classifier<Classifier>);
}

}
}
#endif
