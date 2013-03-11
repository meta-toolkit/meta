#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <unordered_map>
#include <memory>
#include <string>
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"

class Tokenizer;

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
    std::shared_ptr<Tokenizer> create_tokenizer(
        const std::unordered_map<std::string, std::string> & config);

    static const std::unordered_map<std::string, NgramTokenizer::NgramType> ngramOpt = {
        {"POS", NgramTokenizer::POS}, {"Word", NgramTokenizer::Word},
        {"FW", NgramTokenizer::FW}, {"Char", NgramTokenizer::Char},
        {"Lex", NgramTokenizer::Lex}
    };

    static const std::unordered_map<std::string, TreeTokenizer::TreeTokenizerType> treeOpt = {
        {"Subtree", TreeTokenizer::Subtree}, {"Depth", TreeTokenizer::Depth},
        {"Branch", TreeTokenizer::Branch}, {"Tag", TreeTokenizer::Tag},
        {"Skel", TreeTokenizer::Skeleton}, {"Semi", TreeTokenizer::SemiSkeleton}
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



#endif
