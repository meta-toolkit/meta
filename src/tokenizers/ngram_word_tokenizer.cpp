/**
 * @file ngram_tokenizer.cpp
 */

#include "stemmers/porter2_stemmer.h"
#include "io/parser.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_word_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::deque;
using std::string;
using std::unordered_map;
using std::unordered_set;

using index::TermID;
using index::Document;
using io::Parser;

ngram_word_tokenizer::ngram_word_tokenizer(size_t n, StemmerType stemmerType, StopwordType stopwordType):
    ngram_tokenizer(n),
    _stemmerType(stemmerType),
    _stopwords(unordered_set<string>())
{
    if(stopwordType != NoStopwords)
        init_stopwords();
}

void ngram_word_tokenizer::tokenize(Document & document,
        const std::shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    Parser parser(document.getPath() + ".sen", " \n");

    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < n_value() && parser.hasNext(); ++i)
    {
        string next = "";
        do
        {
            next = stop_or_stem(parser.next());
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(mapping(wordified), 1, docFreq);
        ngram.pop_front();
        string next = "";
        do
        {
            next = stop_or_stem(parser.next());
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the last token
    document.increment(mapping(wordify(ngram)), 1, docFreq);
}

string ngram_word_tokenizer::stop_or_stem(const string & str) const
{
    if(_stemmerType == NoStemmer)
    {
        string ret(str);
        std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
        return ret;
    }
    else
        return stemmers::Porter2Stemmer::stem(stemmers::Porter2Stemmer::trim(str));
}

void ngram_word_tokenizer::init_stopwords()
{
    auto config = io::config_reader::read("tokenizer.ini");
    Parser parser(config["stop-words"], "\n");
    while(parser.hasNext())
        _stopwords.insert(stemmers::Porter2Stemmer::stem(parser.next()));
}

}
}
