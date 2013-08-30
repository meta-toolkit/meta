/**
 * @file ngram_word_tokenizer.tcc
 * @author Sean Massung
 */

#include "tokenizers/ngram/ngram_word_tokenizer.h"
#include "util/common.h"
#include "io/parser.h"

namespace meta {
namespace tokenizers {

template <class Stemmer>
ngram_word_tokenizer<Stemmer>::ngram_word_tokenizer(
        size_t n,
        ngram_word_traits::StopwordType stopwordType):
    ngram_tokenizer{n}
{
    if(stopwordType != ngram_word_traits::NoStopwords)
        init_stopwords();
}

template <class Stemmer>
void ngram_word_tokenizer<Stemmer>::tokenize_document(
        index::document & document,
        std::function<term_id(const std::string &)> mapping)
{
    meta::io::parser psr{document.path() + ".sen", " \n"};

    // initialize the ngram
    std::deque<std::string> ngram;
    for(size_t i = 0; i < n_value() && psr.has_next(); ++i)
    {
        std::string next = "";
        do
        {
            next = stem(psr.next());
        } while(_stopwords.find(next) != _stopwords.end() && psr.has_next());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(psr.has_next())
    {
        std::string wordified = wordify(ngram);
        document.increment(mapping(wordified), 1);
        ngram.pop_front();
        std::string next = "";
        do
        {
            next = stem(psr.next());
        } while(_stopwords.find(next) != _stopwords.end() && psr.has_next());
        ngram.push_back(next);
    }

    // add the last token
    document.increment(mapping(wordify(ngram)), 1);
}

template <class Stemmer>
void ngram_word_tokenizer<Stemmer>::init_stopwords()
{
    auto config = common::read_config("config.toml");
    io::parser p{*cpptoml::get_as<std::string>(config, "stop-words"), "\n"};
    while(p.has_next())
        _stopwords.insert(stem(p.next()));
}

}
}
