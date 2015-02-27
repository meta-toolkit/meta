/**
 * @file filter_factory.cpp
 * @author Chase Geigle
 */

#include "analyzers/filter_factory.h"

#include "analyzers/tokenizers/character_tokenizer.h"
#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "analyzers/tokenizers/icu_tokenizer.h"

#include "analyzers/filters/alpha_filter.h"
#include "analyzers/filters/empty_sentence_filter.h"
#include "analyzers/filters/english_normalizer.h"
#include "analyzers/filters/icu_filter.h"
#include "analyzers/filters/length_filter.h"
#include "analyzers/filters/list_filter.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/filters/porter2_stemmer.h"
#include "analyzers/filters/ptb_normalizer.h"
#include "analyzers/filters/sentence_boundary.h"

namespace meta
{
namespace analyzers
{

template <class Tokenizer>
void filter_factory::register_tokenizer()
{
    add(Tokenizer::id,
        [](std::unique_ptr<token_stream> source, const cpptoml::table& config)
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
    register_filter<filters::ptb_normalizer>();
    register_filter<filters::sentence_boundary>();
}
}
}
