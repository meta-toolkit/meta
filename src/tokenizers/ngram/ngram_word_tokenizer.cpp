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

ngram_word_tokenizer::ngram_word_tokenizer(
        uint16_t n,
        stopword_t stopwords,
        std::function<void(std::string &)> stemmer)
: ngram_tokenizer{n}, _stemmer{stemmer}
{
    if(stopwords != stopword_t::None)
        init_stopwords();
}

void ngram_word_tokenizer::tokenize(corpus::document & doc)
{
    io::parser psr{create_parser(doc, ".sen", " \n")};

    // initialize the ngram
    std::deque<std::string> ngram;
    for(size_t i = 0; i < n_value() && psr.has_next(); ++i)
    {
        std::string next = "";
        do
        {
            next = psr.next();
            _stemmer(next);
        } while(_stopwords.find(next) != _stopwords.end() && psr.has_next());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(psr.has_next())
    {
        doc.increment(wordify(ngram), 1);
        ngram.pop_front();
        std::string next = "";
        do
        {
            next = psr.next();
            _stemmer(next);
        } while(_stopwords.find(next) != _stopwords.end() && psr.has_next());
        ngram.push_back(next);
    }

    // add the last token
    doc.increment(wordify(ngram), 1);
}

void ngram_word_tokenizer::init_stopwords()
{
    auto config = cpptoml::parse_file("config.toml");
    io::parser p{*config.get_as<std::string>("stop-words"), "\n"};
    while(p.has_next())
    {
        std::string sword{p.next()};
        _stemmer(sword);
        _stopwords.insert(sword);
    }
}

}
}
