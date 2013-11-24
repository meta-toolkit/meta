/**
 * @file ngram_word_tokenizer.cpp
 * @author Sean Massung
 */

#include "tokenizers/ngram/ngram_word_tokenizer.h"
#include "util/common.h"
#include "io/parser.h"

using namespace std;

namespace meta {
namespace tokenizers {

const std::string ngram_word_tokenizer::_delimiters
    = " \n\t!\"#$%&()*+,-./:;<=>?@[\\]^_`{|}~";

ngram_word_tokenizer::ngram_word_tokenizer(
        uint16_t n,
        stopword_t stopwords,
        std::function<void(std::string &)> stemmer)
: ngram_tokenizer{n}, _stemmer{stemmer}
{
    if(stopwords != stopword_t::None)
        init_stopwords();
}

void ngram_word_tokenizer::tokenize_document(
        corpus::document & doc,
        std::function<term_id(const std::string &)> mapping)
{
    // first, get tokens

    io::parser psr{create_parser(doc, ".sen", _delimiters)};
    std::vector<std::string> tokens;
    std::string token;
    while(psr.has_next())
    {
        token = psr.next();
        _stemmer(token);
        if(_stopwords.find(token) == _stopwords.end())
            tokens.push_back(token);
    }

    // second, create ngrams from them

    for(size_t i = n_value() - 1; i < tokens.size(); ++i)
    {
        std::string combined = tokens[i];
        for(size_t j = 1; j < n_value(); ++j)
            combined = tokens[i - j] + "_" + combined;

        doc.increment(mapping(combined), 1);
    }
}

void ngram_word_tokenizer::init_stopwords()
{
    auto config = cpptoml::parse_file("config.toml");
    io::parser p{*config.get_as<std::string>("stop-words"), "\n"};
    while(p.has_next())
    {
        std::string word{p.next()};
        _stemmer(word);
        _stopwords.insert(word);
    }
}

}
}
