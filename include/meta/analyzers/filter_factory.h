/**
 * @file filter_factory.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FILTER_FACTORY_H_
#define META_FILTER_FACTORY_H_

#include "meta/analyzers/token_stream.h"
#include "meta/config.h"
#include "meta/util/factory.h"
#include "meta/util/shim.h"

namespace cpptoml
{
class table;
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
    : public util::factory<filter_factory, token_stream,
                           std::unique_ptr<token_stream>, const cpptoml::table&>
{
    /// friend the base factory
    friend base_factory;

  private:
    /**
     * Constructor.
     */
    filter_factory();

    /**
     * Adds (registers) a tokenizer with this factory so it is able to be
     * created.
     */
    template <class Tokenizer>
    void register_tokenizer();

    /**
     * Adds (registers) a filter with this factory so it is able to be
     * created.
     */
    template <class Filter>
    void register_filter();
};

namespace tokenizers
{
/**
 * Factory method for creating a tokenizer. This should be specialized if
 * your given tokenizer requires special construction behavior.
 */
template <class Tokenizer>
std::unique_ptr<token_stream> make_tokenizer(const cpptoml::table&)
{
    return make_unique<Tokenizer>();
}
}

namespace filters
{
/**
 * Factory method for creating a filter. This should be specialized if your
 * given filter requires special behavior.
 */
template <class Filter>
std::unique_ptr<token_stream> make_filter(std::unique_ptr<token_stream> source,
                                          const cpptoml::table&)
{
    return make_unique<Filter>(std::move(source));
}
}

/**
 * Registration method for tokenizers. Clients should use this method to
 * register any new tokenizers they write.
 */
template <class Tokenizer>
void register_tokenizer()
{
    filter_factory::get().add(
        Tokenizer::id,
        [](std::unique_ptr<token_stream> source, const cpptoml::table& config) {
            if (source)
                throw typename Tokenizer::token_stream_exception{
                    "tokenizers must be the first filter"};
            return tokenizers::make_tokenizer<Tokenizer>(config);
        });
}

/**
 * Registration method for filters. Clients should use this method to
 * register any new filters they write.
 */
template <class Filter>
void register_filter()
{
    filter_factory::get().add(Filter::id, filters::make_filter<Filter>);
}
}
}
#endif
