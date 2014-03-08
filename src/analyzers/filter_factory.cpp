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
#include "analyzers/filters/sentence_boundary.h"

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
        return make_tokenizer<Tokenizer>(config);
    });
}

template <class Filter>
void filter_factory::register_filter()
{
    add(Filter::id, make_filter<Filter>);
}

filter_factory::filter_factory()
{
    // built-in tokenizers
    register_tokenizer<character_tokenizer>();
    register_tokenizer<whitespace_tokenizer>();
    register_tokenizer<icu_tokenizer>();

    // built-in filters
    register_filter<alpha_filter>();
    register_filter<empty_sentence_filter>();
    register_filter<english_normalizer>();
    register_filter<icu_filter>();
    register_filter<length_filter>();
    register_filter<list_filter>();
    register_filter<lowercase_filter>();
    register_filter<porter2_stemmer>();
    register_filter<sentence_boundary>();
}
}
}
