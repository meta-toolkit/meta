/**
 * @file ngram_fw_tokenizer.cpp
 */

#include "io/parser.h"
#include "util/common.h"
#include "tokenizers/ngram/ngram_fw_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_fw_tokenizer::ngram_fw_tokenizer(uint16_t n):
    ngram_tokenizer{n}
{
    auto config = cpptoml::parse_file("config.toml");
    io::parser parser{*config.get_as<std::string>("function-words"), "\n"};
    while(parser.has_next())
        _function_words.insert(parser.next());
}

void ngram_fw_tokenizer::tokenize(corpus::document & doc)
{
    io::parser parser{create_parser(doc, ".sen", " \n")};

    // initialize the ngram
    std::deque<std::string> ngram;
    for(size_t i = 0; i < n_value() && parser.has_next(); ++i)
    {
        std::string next = "";
        do
        {
            next = parser.next();
        } while(_function_words.find(next) == _function_words.end() && parser.has_next());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.has_next())
    {
        doc.increment(wordify(ngram), 1);
        ngram.pop_front();
        std::string next = "";
        do
        {
            next = parser.next();
        } while(_function_words.find(next) == _function_words.end() && parser.has_next());
        ngram.push_back(next);
    }

    // add the last token
    doc.increment(wordify(ngram), 1);
}

}
}
