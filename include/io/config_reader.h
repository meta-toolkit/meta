/**
 * @file config_reader.h
 */

#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"

namespace meta {
namespace io {

/**
 * A very simple configuration file reader.
 */
namespace config_reader
{
    /**
     * @param path The path to the config file
     * @return a mapping of option name to option value
     */
    std::unordered_map<std::string, std::string> read(const std::string & path);

    /**
     * @return a Tokenizer as specified by a config object
     */
    std::shared_ptr<tokenizers::tokenizer> create_tokenizer(
        const std::unordered_map<std::string, std::string> & config);

    /**
     * Keeps track of which tree option to use depending on config setting.
     */
    static const std::unordered_map<
        std::string, tokenizers::tree_tokenizer::TreeTokenizerType
    > treeOpt = {
        {"Subtree", tokenizers::tree_tokenizer::Subtree},
        {"Depth",   tokenizers::tree_tokenizer::Depth},
        {"Branch",  tokenizers::tree_tokenizer::Branch},
        {"Tag",     tokenizers::tree_tokenizer::Tag},
        {"Skel",    tokenizers::tree_tokenizer::Skeleton},
        {"Semi",    tokenizers::tree_tokenizer::SemiSkeleton}
    };

    /**
     * Basic exception for config_reader interactions.
     */
    class config_reader_exception: public std::exception
    {
        public:
            
            config_reader_exception(const std::string & error):
                _error(error) { /* nothing */ }

            const char* what () const throw ()
            {
                return _error.c_str();
            }
       
        private:
       
            std::string _error;
    };
}

}
}

#endif
