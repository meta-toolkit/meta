/**
 * @file analyzer.cpp
 */

#include "analyzers/analyzer_factory.h"
#include "analyzers/filter_factory.h"
#include "analyzers/multi_analyzer.h"
#include "analyzers/token_stream.h"
#include "analyzers/filters/alpha_filter.h"
#include "analyzers/filters/empty_sentence_filter.h"
#include "analyzers/filters/length_filter.h"
#include "analyzers/filters/list_filter.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/filters/porter2_stemmer.h"
#include "analyzers/tokenizers/icu_tokenizer.h"
#include "corpus/document.h"
#include "cpptoml.h"
#include "io/mmap_file.h"
#include "util/shim.h"
#include "utf/utf.h"

namespace meta
{
namespace analyzers
{

std::string analyzer::get_content(const corpus::document& doc)
{
    if (doc.contains_content())
        return utf::to_utf8(doc.content(), doc.encoding());

    io::mmap_file file{doc.path()};
    return utf::to_utf8({file.begin(), file.size()}, doc.encoding());
}

io::parser analyzer::create_parser(const corpus::document& doc,
                                   const std::string& extension,
                                   const std::string& delims)
{
    if (doc.contains_content())
        return io::parser{doc.content(), delims,
                          io::parser::input_type::String};
    else
        return io::parser{doc.path() + extension, delims,
                          io::parser::input_type::File};
}

std::unique_ptr<token_stream>
    analyzer::default_filter_chain(const cpptoml::table& config)
{

    auto stopwords = config.get_as<std::string>("stop-words");

    std::unique_ptr<token_stream> result;

    result = make_unique<tokenizers::icu_tokenizer>();
    result = make_unique<filters::lowercase_filter>(std::move(result));
    result = make_unique<filters::alpha_filter>(std::move(result));
    result = make_unique<filters::length_filter>(std::move(result), 2, 35);
    result = make_unique<filters::list_filter>(std::move(result), *stopwords);
    result = make_unique<filters::porter2_stemmer>(std::move(result));
    result = make_unique<filters::empty_sentence_filter>(std::move(result));
    return result;
}

std::unique_ptr<token_stream>
    analyzer::load_filter(std::unique_ptr<token_stream> src,
                          const cpptoml::table& config)
{
    auto type = config.get_as<std::string>("type");
    if (!type)
        throw analyzer_exception{"filter type missing in config file"};
    return filter_factory::get().create(*type, std::move(src), config);
}

std::unique_ptr<token_stream>
    analyzer::load_filters(const cpptoml::table& global,
                           const cpptoml::table& config)
{

    auto check = config.get_as<std::string>("filter");
    if (check)
    {
        if (*check == "default-chain")
            return default_filter_chain(global);
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

std::unique_ptr<analyzer> analyzer::load(const cpptoml::table& config)
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
