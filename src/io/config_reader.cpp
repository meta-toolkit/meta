#include <fstream>
#include "tokenizers/tokenizers.h"

namespace meta {
namespace io {

using std::shared_ptr;
using std::make_shared;
using std::string;
using std::unordered_map;

using tokenizers::tokenizer;
using tokenizers::multi_tokenizer;
using tokenizers::tree_tokenizer;
using tokenizers::ngram_tokenizer;

string config_reader::get_config_string( const cpptoml::toml_group & config ) {
    std::stringstream ss;
    ss << "config";
    ss << "-" << *cpptoml::get_as<std::string>( config, "dataset" );
    ss << "-" << *cpptoml::get_as<std::string>( config, "list" );
    auto tokenizers = config.get_group_array( "tokenizers" );
    for( auto group : tokenizers->array() ) {
        for( auto p : *group ) {
            ss << "-";
            p.second->print( ss );
        }
    }
    return ss.str();
}

cpptoml::toml_group config_reader::read( const string & path ) {
    std::ifstream file{ path };
    if( !file.is_open() )
        throw config_reader_exception{ "Failed to open " + path };
    cpptoml::parser p{ file };
    return p.parse();
}

shared_ptr<tokenizer> config_reader::create_tokenizer( const cpptoml::toml_group & config ) {
    std::vector<shared_ptr<tokenizer>> toks;
    
    auto tokenizers = config.get_group_array( "tokenizers" );
    for( auto group : tokenizers->array() ) {
        string method = *cpptoml::get_as<std::string>( *group, "method" );
        if( method == "tree" ) {
            string type = *cpptoml::get_as<std::string>( *group, "treeOpt" );
            if( type == "Branch" )
                toks.emplace_back( make_shared<tokenizers::branch_tokenizer>() );
            else if( type == "Depth" )
                toks.emplace_back( make_shared<tokenizers::depth_tokenizer>() );
            else if( type == "Semi" )
                toks.emplace_back( make_shared<tokenizers::semi_skeleton_tokenizer>() );
            else if( type == "Skel" )
                toks.emplace_back( make_shared<tokenizers::skeleton_tokenizer>() );
            else if( type == "Subtree" )
                toks.emplace_back( make_shared<tokenizers::subtree_tokenizer>() );
            else if( type == "Tag" )
                toks.emplace_back( make_shared<tokenizers::tag_tokenizer>() );
            else
                throw config_reader_exception{ "tree method was not able to be determined" };
        } else if( method == "ngram" ) {
            int64_t n_val = *cpptoml::get_as<int64_t>( *group, "ngram" );
            string type = *cpptoml::get_as<std::string>( *group, "ngramOpt" );
            if( type == "Word" )
                toks.emplace_back( make_shared<tokenizers::ngram_word_tokenizer<>>( n_val ) );
            else if( type == "FW" )
                toks.emplace_back( make_shared<tokenizers::ngram_fw_tokenizer>( n_val ) );
            else if( type == "Lex" )
                toks.emplace_back( make_shared<tokenizers::ngram_lex_tokenizer>( n_val ) );
            else if( type == "POS" )
                toks.emplace_back( make_shared<tokenizers::ngram_pos_tokenizer>( n_val ) );
            else if( type == "Char" )
                toks.emplace_back( make_shared<tokenizers::ngram_char_tokenizer>( n_val ) );
            else
                throw config_reader_exception{ "ngram method was not able to be determined" };
        } else {
            throw config_reader_exception{ "method was not able to be determined" };
        }
    }
    return make_shared<multi_tokenizer>( toks );
}

}
}
