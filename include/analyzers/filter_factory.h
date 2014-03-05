/**
 * @file filter_factory.h
 * @author Chase Geigle
 */

#ifndef _META_FILTER_FACTORY_H_
#define _META_FILTER_FACTORY_H_

#include <functional>
#include <memory>
#include <unordered_map>

#include "analyzers/token_stream.h"
#include "util/shim.h"

namespace cpptoml
{
class toml_group;
}

namespace meta
{
namespace analyzers
{

/**
 * Factory that is responsible for creating filters during analyzer
 * construction. Clients should use the register_X methods instead of this
 * class directly.
 */
class filter_factory
{
  public:
    using pointer = std::unique_ptr<token_stream>;
    using factory_method =
        std::function<pointer(pointer, const cpptoml::toml_group&)>;
    using exception = token_stream::token_stream_exception;

    /**
     * Obtains the singleton.
     */
    inline static filter_factory& get()
    {
        static filter_factory factory;
        return factory;
    }

    /**
     * Associates the given identifier with the given factory method.
     */
    template <class Function>
    void add(const std::string& identifier, Function&& fn)
    {
        if (methods_.find(identifier) != methods_.end())
            throw exception{"filter already registered with that id"};
        methods_.emplace(identifier, std::forward<Function>(fn));
    }

    /**
     * Creates a new filter based on the identifier, a source to read
     * tokens from, and a configuration object.
     */
    pointer create(const std::string& identifier, pointer source,
                   const cpptoml::toml_group& config);
  private:
    filter_factory();

    template <class Tokenizer>
    void register_tokenizer();

    template <class Filter>
    void register_filter();

    std::unordered_map<std::string, factory_method> methods_;
};

/**
 * Factory method for creating a tokenizer. This should be specialized if
 * your given tokenizer requires special construction behavior.
 */
template <class Tokenizer>
std::unique_ptr<token_stream> make_tokenizer(const cpptoml::toml_group&)
{
    return make_unique<Tokenizer>();
}

/**
 * Factory method for creating a filter. This should be specialized if your
 * given filter requires special behavior.
 */
template <class Filter>
std::unique_ptr<token_stream> make_filter(std::unique_ptr<token_stream> source,
                                          const cpptoml::toml_group&)
{
    return make_unique<Filter>(std::move(source));
}

/**
 * Registration method for tokenizers. Clients should use this method to
 * register any new tokenizers they write.
 */
template <class Tokenizer>
void register_tokenizer()
{
    filter_factory::get().add(Tokenizer::id,
                              [](std::unique_ptr<token_stream> source,
                                 const cpptoml::toml_group& config)
    {
        if (source)
            throw typename Tokenizer::token_stream_exception{
                "tokenizers must be the first filter"};
        return make_tokenizer<Tokenizer>(config);
    });
}

/**
 * Registration method for filters. Clients should use this method to
 * register any new filters they write.
 */
template <class Filter>
void register_filter()
{
    filter_factory::get().add(Filter::id, make_filter<Filter>);
}

}
}
#endif
