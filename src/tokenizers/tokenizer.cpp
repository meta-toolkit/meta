/**
 * @file tokenizer.cpp
 */

#include <functional>
#include <fstream>
#include <iostream>
#include "util/common.h"
#include "util/invertible_map.h"
#include "stemmers/no_stemmer.h"
#include "stemmers/porter2.h"
#include "tokenizers/tokenizer.h"
#include "tokenizers/all.h"

namespace meta {
namespace tokenizers {

using namespace std::placeholders;

tokenizer::tokenizer():
    _current_term_id{0}
{ /* nothing */ }

void tokenizer::tokenize(corpus::document & document)
{
    tokenize_document(document, std::bind(&tokenizer::mapping, this, _1));
}

term_id tokenizer::mapping(const std::string & term)
{
    std::lock_guard<std::mutex> lock{mutables_};
    if(!_term_map.contains_value(term))
    {
        _term_map.insert(_current_term_id, term);
        return _current_term_id++;
    }
    else
    {
        term_id termID = _term_map.get_key(term);
        return termID;
    }
}

void tokenizer::set_term_id_mapping(const std::string & filename)
{
    common::load_mapping(_term_map, filename);
    _current_term_id = _term_map.size();
}

void tokenizer::save_term_id_mapping(const std::string & filename) const
{
    common::save_mapping(_term_map, filename);
}

const util::invertible_map<term_id, std::string> & tokenizer::term_id_mapping() const
{
    return _term_map;
}

std::string tokenizer::label(term_id termID) const
{
    return _term_map.get_value(termID);
}

void tokenizer::set_max_term_id(size_t start)
{
    _current_term_id = start;
}

term_id tokenizer::max_term_id() const
{
    return _current_term_id;
}

size_t tokenizer::num_terms() const
{
    return _term_map.size();

}

io::parser tokenizer::create_parser(const corpus::document & doc,
        const std::string & extension, const std::string & delims)
{
    if(doc.contains_content())
        return io::parser{doc.content(),
                          delims,
                          io::parser::input_type::String};
    else
        return io::parser{doc.path() + extension,
                          delims,
                          io::parser::input_type::File};
}

std::unique_ptr<tokenizer> tokenizer::load_tokenizer(const cpptoml::toml_group & config)
{
    std::vector<std::shared_ptr<tokenizer>> toks;
    auto tokenizers = config.get_group_array( "tokenizers" );
    for( auto group : tokenizers->array() ) {
        auto method = group->get_as<std::string>("method");
        if (!method)
            throw tokenizer_exception{"failed to find tokenizer method"};
        if( *method == "tree" ) {
            auto type = group->get_as<std::string>("treeOpt");
            if (!type)
                throw tokenizer_exception{"tree method needed in config file"};
            if( *type == "Branch" )
                toks.emplace_back( std::make_shared<tokenizers::branch_tokenizer>() );
            else if( *type == "Depth" )
                toks.emplace_back( std::make_shared<tokenizers::depth_tokenizer>() );
            else if( *type == "Semi" )
                toks.emplace_back( std::make_shared<tokenizers::semi_skeleton_tokenizer>() );
            else if( *type == "Skel" )
                toks.emplace_back( std::make_shared<tokenizers::skeleton_tokenizer>() );
            else if( *type == "Subtree" )
                toks.emplace_back( std::make_shared<tokenizers::subtree_tokenizer>() );
            else if( *type == "Tag" )
                toks.emplace_back( std::make_shared<tokenizers::tag_tokenizer>() );
            else
                throw tokenizer_exception{ "tree method was not able to be determined" };
        } else if( *method == "ngram" ) {

            auto n_val = group->get_as<int64_t>("ngram");
            if (!n_val)
                throw tokenizer_exception{"ngram size needed in config file"};

            auto type = group->get_as<std::string>("ngramOpt");
            if (!type)
                throw tokenizer_exception{"ngram type needed in config file"};

            if( *type == "Word" ) {
                // determine stemmer type
                auto stem = group->get_as<std::string>("stemmer");
                std::function<void(std::string &)> stemmer =
                    stemmers::porter2{};
                if (stem && *stem == "None")
                    stemmer = stemmers::no_stemmer{};

                // determine whether or not to use stopwords
                auto stop = group->get_as<bool>("stopwords");
                auto stopwords =
                    tokenizers::ngram_word_tokenizer::stopword_t::Default;
                if (stop && !*stop)
                    stopwords = tokenizers::ngram_word_tokenizer::stopword_t::None;

                auto tok = std::make_shared<tokenizers::ngram_word_tokenizer>(
                        *n_val, stopwords, stemmer);
                toks.emplace_back(std::move(tok));
            } else if( *type == "FW" ) {
                toks.emplace_back( std::make_shared<tokenizers::ngram_fw_tokenizer>( *n_val ) );
            } else if( *type == "Lex" ) {
                toks.emplace_back( std::make_shared<tokenizers::ngram_lex_tokenizer>( *n_val ) );
            } else if( *type == "POS" ) {
                toks.emplace_back( std::make_shared<tokenizers::ngram_pos_tokenizer>( *n_val ) );
            } else if( *type == "Char" ) {
                toks.emplace_back( std::make_shared<tokenizers::ngram_char_tokenizer>( *n_val ) );
            } else {
                throw tokenizer_exception{ "ngram method was not able to be determined" };
            }
        } else if (*method == "libsvm") {
            toks.emplace_back( std::make_shared<tokenizers::libsvm_tokenizer>() );
        } else {
            throw tokenizer_exception{ "method was not able to be determined" };
        }
    }
    return common::make_unique<multi_tokenizer>(toks);
}

}
}
