/**
 * @file analyzer.cpp
 */

#include "analyzers/all.h"
#include "cpptoml.h"
#include "corpus/document.h"
#include "io/parser.h"
#include "stemmers/no_stemmer.h"
#include "stemmers/porter2.h"
#include "util/shim.h"

namespace meta {
namespace analyzers {

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

std::unique_ptr<analyzer> analyzer::load(const cpptoml::toml_group & config)
{
    using namespace analyzers;
    std::vector<std::shared_ptr<analyzer>> toks;
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
                toks.emplace_back(std::make_shared<branch_analyzer>());
            else if(*type == "Depth")
                toks.emplace_back(std::make_shared<depth_analyzer>());
            else if(*type == "Semi")
                toks.emplace_back(std::make_shared<semi_skeleton_analyzer>());
            else if(*type == "Skel")
                toks.emplace_back(std::make_shared<skeleton_analyzer>());
            else if(*type == "Subtree")
                toks.emplace_back(std::make_shared<subtree_analyzer>());
            else if(*type == "Tag")
                toks.emplace_back(std::make_shared<tag_analyzer>());
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
            {
                // determine stemmer type
                auto stem = group->get_as<std::string>("stemmer");
                std::function<void(std::string &)> stemmer =
                    stemmers::porter2{};
                if(stem && *stem == "None")
                    stemmer = stemmers::no_stemmer{};

                // determine whether or not to use stopwords
                auto stop = group->get_as<bool>("stopwords");
                auto stopwords = ngram_word_analyzer::stopword_t::Default;
                if(stop && !*stop)
                    stopwords = ngram_word_analyzer::stopword_t::None;

                auto tok = std::make_shared<ngram_word_analyzer>(
                        *n_val, stopwords, stemmer);
                toks.emplace_back(std::move(tok));
            }
            else if(*type == "FW")
              toks.emplace_back(std::make_shared<ngram_fw_analyzer>(*n_val));
            else if(*type == "Lex")
              toks.emplace_back(std::make_shared<ngram_lex_analyzer>(*n_val));
            else if(*type == "POS")
              toks.emplace_back(std::make_shared<ngram_pos_analyzer>(*n_val));
            else if(*type == "Char")
              toks.emplace_back(std::make_shared<ngram_char_analyzer>(*n_val));
            else
              throw analyzer_exception{
                  "ngram method was not able to be determined"};
        }
        else if(*method == "libsvm")
            toks.emplace_back(std::make_shared<libsvm_analyzer>());
        else
            throw analyzer_exception{"method was not able to be determined"};
    }
    return make_unique<multi_analyzer>(toks);
}

}
}
