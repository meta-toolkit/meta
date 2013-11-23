/**
 * @file tokenizer.cpp
 */

#include "util/common.h"
#include "stemmers/no_stemmer.h"
#include "stemmers/porter2.h"
#include "tokenizers/tokenizer.h"
#include "tokenizers/all.h"

namespace meta {
namespace tokenizers {

io::parser tokenizer::create_parser(const corpus::document & doc,
        const std::string & extension, const std::string & delims)
{
    if(doc.contains_content())
        return io::parser{doc.content(), delims,
                          io::parser::input_type::String};
    else
        return io::parser{doc.path() + extension, delims,
                          io::parser::input_type::File};
}

std::unique_ptr<tokenizer> tokenizer::load_tokenizer(
        const cpptoml::toml_group & config)
{
    using namespace tokenizers;
    std::vector<std::shared_ptr<tokenizer>> toks;
    auto tokenizers = config.get_group_array("tokenizers");
    for(auto group: tokenizers->array())
    {
        auto method = group->get_as<std::string>("method");
        if(!method)
            throw tokenizer_exception{"failed to find tokenizer method"};
        if(*method == "tree")
        {
            auto type = group->get_as<std::string>("treeOpt");
            if(!type)
                throw tokenizer_exception{"tree method needed in config file"};
            if(*type == "Branch")
                toks.emplace_back(std::make_shared<branch_tokenizer>());
            else if(*type == "Depth")
                toks.emplace_back(std::make_shared<depth_tokenizer>());
            else if(*type == "Semi")
                toks.emplace_back(std::make_shared<semi_skeleton_tokenizer>());
            else if(*type == "Skel")
                toks.emplace_back(std::make_shared<skeleton_tokenizer>());
            else if(*type == "Subtree")
                toks.emplace_back(std::make_shared<subtree_tokenizer>());
            else if(*type == "Tag")
                toks.emplace_back(std::make_shared<tag_tokenizer>());
            else
                throw tokenizer_exception{
                    "tree method was not able to be determined"};
        }
        else if(*method == "ngram")
        {
            auto n_val = group->get_as<int64_t>("ngram");
            if(!n_val)
                throw tokenizer_exception{"ngram size needed in config file"};

            auto type = group->get_as<std::string>("ngramOpt");
            if(!type)
                throw tokenizer_exception{"ngram type needed in config file"};

            if(*type == "Word")
            {
                // determine stemmer type
                auto stem = group->get_as<std::string>("stemmer");
                std::function<std::string(const std::string &)> stemmer =
                    stemmers::porter2{};
                if(stem && *stem == "None")
                    stemmer = stemmers::no_stemmer{};

                // determine whether or not to use stopwords
                auto stop = group->get_as<bool>("stopwords");
                auto stopwords = ngram_word_tokenizer::stopword_t::Default;
                if(stop && !*stop)
                    stopwords = ngram_word_tokenizer::stopword_t::None;

                auto tok = std::make_shared<ngram_word_tokenizer>(
                        *n_val, stopwords, stemmer);
                toks.emplace_back(std::move(tok));
            }
            else if(*type == "FW")
              toks.emplace_back(std::make_shared<ngram_fw_tokenizer>(*n_val));
            else if(*type == "Lex")
              toks.emplace_back(std::make_shared<ngram_lex_tokenizer>(*n_val));
            else if(*type == "POS")
              toks.emplace_back(std::make_shared<ngram_pos_tokenizer>(*n_val));
            else if(*type == "Char")
              toks.emplace_back(std::make_shared<ngram_char_tokenizer>(*n_val));
            else
              throw tokenizer_exception{
                  "ngram method was not able to be determined"};
        }
        else if(*method == "libsvm")
            toks.emplace_back(std::make_shared<libsvm_tokenizer>());
        else
            throw tokenizer_exception{"method was not able to be determined"};
    }
    return common::make_unique<multi_tokenizer>(toks);
}

}
}
