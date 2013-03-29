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
namespace ConfigReader
{
    /**
     * @param path The path to the config file
     * @return a mapping of option name to option value
     */
    std::unordered_map<std::string, std::string> read(const std::string & path);

    /**
     * @return a Tokenizer as specified by a config object
     */
    std::shared_ptr<tokenizers::Tokenizer> create_tokenizer(
        const std::unordered_map<std::string, std::string> & config);

    static const std::unordered_map<
        std::string, tokenizers::NgramTokenizer::NgramType
    > ngramOpt = {
        {"POS",  tokenizers::NgramTokenizer::POS},
        {"Word", tokenizers::NgramTokenizer::Word},
        {"FW",   tokenizers::NgramTokenizer::FW},
        {"Char", tokenizers::NgramTokenizer::Char},
        {"Lex",  tokenizers::NgramTokenizer::Lex}
    };

    static const std::unordered_map<
        std::string, tokenizers::TreeTokenizer::TreeTokenizerType
    > treeOpt = {
        {"Subtree", tokenizers::TreeTokenizer::Subtree},
        {"Depth",   tokenizers::TreeTokenizer::Depth},
        {"Branch",  tokenizers::TreeTokenizer::Branch},
        {"Tag",     tokenizers::TreeTokenizer::Tag},
        {"Skel",    tokenizers::TreeTokenizer::Skeleton},
        {"Semi",    tokenizers::TreeTokenizer::SemiSkeleton}
    };
}

/**
 * Basic exception for ConfigReader interactions.
 */
class ConfigReaderException: public std::exception
{
    public:
        
        ConfigReaderException(const std::string & error):
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

#endif
