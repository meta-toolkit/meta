/**
 * @file analyzer.cpp
 */

#include "meta/analyzers/analyzer_factory.h"
#include "meta/analyzers/filter_factory.h"
#include "meta/analyzers/multi_analyzer.h"
#include "meta/analyzers/token_stream.h"
#include "meta/analyzers/filters/alpha_filter.h"
#include "meta/analyzers/filters/empty_sentence_filter.h"
#include "meta/analyzers/filters/length_filter.h"
#include "meta/analyzers/filters/list_filter.h"
#include "meta/analyzers/filters/lowercase_filter.h"
#include "meta/analyzers/filters/porter2_filter.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/corpus/document.h"
#include "cpptoml.h"
#include "meta/io/mmap_file.h"
#include "meta/util/shim.h"
#include "meta/utf/utf.h"

namespace meta
{
namespace analyzers
{

std::string get_content(const corpus::document& doc)
{
    if (!doc.contains_content())
        throw analyzer_exception{
            "document content was not populated for analysis"};

    return utf::to_utf8(doc.content(), doc.encoding());
}

namespace
{
std::unique_ptr<token_stream>
add_default_filters(std::unique_ptr<token_stream> tokenizer,
                    const cpptoml::table& config)
{
    auto stopwords = config.get_as<std::string>("stop-words");

    std::unique_ptr<token_stream> result;

    result = make_unique<filters::lowercase_filter>(std::move(tokenizer));
    result = make_unique<filters::alpha_filter>(std::move(result));
    result = make_unique<filters::length_filter>(std::move(result), 2, 35);
    result = make_unique<filters::list_filter>(std::move(result), *stopwords);
    result = make_unique<filters::porter2_filter>(std::move(result));
    return result;
}
}

std::unique_ptr<token_stream> default_filter_chain(const cpptoml::table& config)
{
    auto tokenizer = make_unique<tokenizers::icu_tokenizer>();
    auto result = add_default_filters(std::move(tokenizer), config);
    result = make_unique<filters::empty_sentence_filter>(std::move(result));
    return result;
}

std::unique_ptr<token_stream>
default_unigram_chain(const cpptoml::table& config)
{
    // suppress "<s>", "</s>"
    auto tokenizer = make_unique<tokenizers::icu_tokenizer>(true);
    return add_default_filters(std::move(tokenizer), config);
}

std::unique_ptr<token_stream> load_filter(std::unique_ptr<token_stream> src,
                                          const cpptoml::table& config)
{
    auto type = config.get_as<std::string>("type");
    if (!type)
        throw analyzer_exception{"filter type missing in config file"};
    return filter_factory::get().create(*type, std::move(src), config);
}

std::unique_ptr<token_stream> load_filters(const cpptoml::table& global,
                                           const cpptoml::table& config)
{

    auto check = config.get_as<std::string>("filter");
    if (check)
    {
        if (*check == "default-chain")
            return default_filter_chain(global);
        else if (*check == "default-unigram-chain")
            return default_unigram_chain(global);
        else
            throw analyzer_exception{"unknown filter option: " + *check};
    }

    auto filters = config.get_table_array("filter");
    if (!filters)
        throw analyzer_exception{"analyzer group missing filter configuration"};
    std::unique_ptr<token_stream> result;
    for (const auto filter : filters->get())
        result = load_filter(std::move(result), *filter);
    return result;
}

std::unique_ptr<analyzer> load(const cpptoml::table& config)
{
    using namespace analyzers;
    std::vector<std::unique_ptr<analyzer>> toks;
    auto analyzers = config.get_table_array("analyzers");
    for (auto group : analyzers->get())
    {
        auto method = group->get_as<std::string>("method");
        if (!method)
            throw analyzer_exception{"failed to find analyzer method"};
        toks.emplace_back(
            analyzer_factory::get().create(*method, config, *group));
    }
    return make_unique<multi_analyzer>(std::move(toks));
}
}
}
