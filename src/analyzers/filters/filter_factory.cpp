/**
 * @file filter_factory.cpp
 * @author Chase Geigle
 */

#include "analyzers/filter_factory.h"

#include "analyzers/tokenizers/character_tokenizer.h"
#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "analyzers/tokenizers/icu_tokenizer.h"
#include "analyzers/filters/all.h"

namespace meta
{
namespace analyzers
{

template <class Tokenizer>
void filter_factory::register_tokenizer()
{
    add(Tokenizer::id, [](std::unique_ptr<token_stream> source,
                          const cpptoml::toml_group& config)
    {
        if (source)
            throw typename Tokenizer::token_stream_exception{
                "tokenizers must be the first filter"};
        return tokenizers::make_tokenizer<Tokenizer>(config);
    });
}

template <class Filter>
void filter_factory::register_filter()
{
    add(Filter::id, filters::make_filter<Filter>);
}

filter_factory::filter_factory()
{
    // built-in tokenizers
    register_tokenizer<tokenizers::character_tokenizer>();
    register_tokenizer<tokenizers::whitespace_tokenizer>();
    register_tokenizer<tokenizers::icu_tokenizer>();

    // built-in filters
    register_filter<filters::alpha_filter>();
    register_filter<filters::empty_sentence_filter>();
    register_filter<filters::english_normalizer>();
    register_filter<filters::icu_filter>();
    register_filter<filters::length_filter>();
    register_filter<filters::list_filter>();
    register_filter<filters::lowercase_filter>();
    register_filter<filters::porter2_stemmer>();
    register_filter<filters::sentence_boundary>();
    register_filter<filters::blank_filter>();
}
}
}
