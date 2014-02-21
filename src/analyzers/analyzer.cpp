/**
 * @file analyzer.cpp
 */

#include "analyzers/all.h"
#include "analyzers/token_stream.h"
#include "analyzers/filters/alpha_filter.h"
#include "analyzers/filters/english_normalizer.h"
#include "analyzers/filters/sentence_boundary.h"
#include "analyzers/filters/length_filter.h"
#include "analyzers/filters/list_filter.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/filters/porter2_stemmer.h"
#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "analyzers/tokenizers/character_tokenizer.h"
#include "cpptoml.h"
#include "corpus/document.h"
#include "io/mmap_file.h"
#include "stemmers/no_stemmer.h"
#include "stemmers/porter2.h"
#include "util/shim.h"

namespace meta {
namespace analyzers {

std::string analyzer::get_content(const corpus::document & doc)
{
    if(doc.contains_content())
        return doc.content();

    io::mmap_file file{doc.path()};
    return {file.begin(), file.size()};
}

io::parser analyzer::create_parser(const corpus::document & doc,
        const std::string & extension, const std::string & delims)
{
    if(doc.contains_content())
        return io::parser{doc.content(), delims,
                          io::parser::input_type::String};
    else
        return io::parser{doc.path() + extension, delims,
                          io::parser::input_type::File};
}

std::unique_ptr<token_stream>
    analyzer::default_filter_chain(const cpptoml::toml_group& config)
{

    auto stopwords = config.get_as<std::string>("stop-words");

    std::unique_ptr<token_stream> result;

    result = make_unique<whitespace_tokenizer>();
    result = make_unique<english_normalizer>(std::move(result));
    sentence_boundary::load_heuristics(config);
    result = make_unique<sentence_boundary>(std::move(result));
    result = make_unique<lowercase_filter>(std::move(result));
    result = make_unique<alpha_filter>(std::move(result));
    result = make_unique<length_filter>(std::move(result), 2, 35);
    result = make_unique<list_filter>(std::move(result), *stopwords);
    result = make_unique<porter2_stemmer>(std::move(result));

    return result;
}

std::unique_ptr<token_stream>
    analyzer::load_filters(const cpptoml::toml_group& global,
                           const cpptoml::toml_group& config)
{

    auto check = config.get_as<std::string>("filter");
    if (check)
    {
        if (*check == "default-chain")
            return default_filter_chain(global);
        else
            throw analyzer_exception{"unknown filter option: " + *check};
    }

    auto filters = config.get_group_array("filter");
    std::unique_ptr<token_stream> result;
    for (const auto filter: filters->array())
    {
        auto type = filter->get_as<std::string>("type");
        if (!type)
            throw analyzer_exception{"filter type missing in config file"};

        if (*type == "whitespace-tokenizer")
        {
            if (result)
                throw analyzer_exception{"tokenizers must be the first filter"};
            result = make_unique<whitespace_tokenizer>();
        }
        else if (*type == "character-tokenizer")
        {
            if (result)
                throw analyzer_exception{"tokenizers must be the first filter"};
            result = make_unique<character_tokenizer>();
        }
        else if (*type == "normalize")
        {
            result = make_unique<english_normalizer>(std::move(result));
        }
        else if (*type == "sentence-boundary")
        {
            sentence_boundary::load_heuristics(*filter);
            result = make_unique<sentence_boundary>(std::move(result));
        }
        else if (*type == "length")
        {
            auto min = filter->get_as<int64_t>("min");
            if (!min)
                throw analyzer_exception{
                    "min required for length filter config"};
            auto max = filter->get_as<int64_t>("max");
            if (!max)
                throw analyzer_exception{
                    "max required for length filter config"};
            result = make_unique<length_filter>(std::move(result),
                                                static_cast<uint64_t>(*min),
                                                static_cast<uint64_t>(*max));
        }
        else if (*type == "list")
        {
            auto method = filter->get_as<std::string>("method");
            auto file = filter->get_as<std::string>("file");
            if (!file)
                throw analyzer_exception{
                    "file required for list_filter config"};

            list_filter::type type = list_filter::type::REJECT;
            if (method)
            {
                if (*method == "accept")
                    type = list_filter::type::ACCEPT;
                else if (*method != "reject")
                    throw analyzer_exception{"invalid method for list_filter"};
            }

            result = make_unique<list_filter>(std::move(result), *file, type);
        }
        else if (*type == "lowercase")
        {
            result = make_unique<lowercase_filter>(std::move(result));
        }
        else if (*type == "porter2-stemmer")
        {
            result = make_unique<porter2_stemmer>(std::move(result));
        }
        else
        {
            throw analyzer_exception{"unrecognized filter option"};
        }
    }
    return result;
}

std::unique_ptr<analyzer> analyzer::load(const cpptoml::toml_group & config)
{
    using namespace analyzers;
    std::vector<std::unique_ptr<analyzer>> toks;
    auto analyzers = config.get_group_array("analyzers");
    for(auto group: analyzers->array())
    {
        auto method = group->get_as<std::string>("method");
        if(!method)
            throw analyzer_exception{"failed to find analyzer method"};
        if(*method == "tree")
        {
            auto type = group->get_as<std::string>("treeOpt");
            if(!type)
                throw analyzer_exception{"tree method needed in config file"};
            if(*type == "Branch")
                toks.emplace_back(make_unique<branch_analyzer>());
            else if(*type == "Depth")
                toks.emplace_back(make_unique<depth_analyzer>());
            else if(*type == "Semi")
                toks.emplace_back(make_unique<semi_skeleton_analyzer>());
            else if(*type == "Skel")
                toks.emplace_back(make_unique<skeleton_analyzer>());
            else if(*type == "Subtree")
                toks.emplace_back(make_unique<subtree_analyzer>());
            else if(*type == "Tag")
                toks.emplace_back(make_unique<tag_analyzer>());
            else
                throw analyzer_exception{
                    "tree method was not able to be determined"};
        }
        else if(*method == "ngram")
        {
            auto n_val = group->get_as<int64_t>("ngram");
            if(!n_val)
                throw analyzer_exception{"ngram size needed in config file"};

            auto type = group->get_as<std::string>("ngramOpt");
            if(!type)
                throw analyzer_exception{"ngram type needed in config file"};

            if(*type == "Word")
                toks.emplace_back(make_unique<ngram_word_analyzer>(
                    *n_val, load_filters(config, *group)));
            else if(*type == "FW")
              toks.emplace_back(make_unique<ngram_fw_analyzer>(*n_val));
            else if(*type == "Lex")
              toks.emplace_back(make_unique<ngram_lex_analyzer>(*n_val));
            else if(*type == "POS")
              toks.emplace_back(make_unique<ngram_pos_analyzer>(*n_val));
            else if(*type == "Char")
              toks.emplace_back(make_unique<ngram_char_analyzer>(*n_val));
            else
              throw analyzer_exception{
                  "ngram method was not able to be determined"};
        }
        else if(*method == "libsvm")
            toks.emplace_back(make_unique<libsvm_analyzer>());
        else
            throw analyzer_exception{"method was not able to be determined"};
    }
    return make_unique<multi_analyzer>(std::move(toks));
}

}
}
