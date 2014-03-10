/**
 * @file filter_factory.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_FILTER_FACTORY_H_
#define _META_FILTER_FACTORY_H_

#include "analyzers/token_stream.h"
#include "util/factory.h"
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
class filter_factory : public util::factory<filter_factory, token_stream,
                                            std::unique_ptr<token_stream>,
                                            const cpptoml::toml_group&>
{
    friend base_factory;

  private:
    filter_factory();

    template <class Tokenizer>
    void register_tokenizer();

    template <class Filter>
    void register_filter();
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
