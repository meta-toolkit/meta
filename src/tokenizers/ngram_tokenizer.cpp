/**
 * @file ngram_tokenizer.cpp
 */

#include <cstdlib>
#include "util/common.h"
#include "stemmers/porter2_stemmer.h"
#include "io/parser.h"
#include "io/config_reader.h"
#include "tokenizers/parse_tree.h"
#include "tokenizers/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::deque;
using std::string;
using std::unordered_map;
using std::unordered_set;

using index::TermID;
using index::Document;
using io::Parser;

NgramTokenizer::NgramTokenizer(size_t n, NgramType ngramType,
                               StemmerType stemmerType, StopwordType stopwordType):
    _nValue(n),
    _ngramType(ngramType),
    _stemmerType(stemmerType),
    _stopwords(unordered_set<string>()), 
    _functionWords(unordered_set<string>())
{
   if(_ngramType == Word && stopwordType != NoStopwords)
       initStopwords();
   else if(_ngramType == FW)
       initFunctionWords();
}

void NgramTokenizer::tokenize(Document & document,
        const std::shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    if(_ngramType == FW)
        tokenizeFW(document, docFreq);
    else if(_ngramType == Word)
        tokenizeWord(document, docFreq);
    else if(_ngramType == Char)
    {
        Parser parser(document.getPath() + ".sen", "");
        tokenizeSimple(document, parser, docFreq);
    }
    else if(_ngramType == Lex)
    {
        Parser parser(document.getPath() + ".lex", "\n");
        tokenizeSimple(document, parser, docFreq);
    }
    else // _ngramType == POS
    {
        Parser parser(document.getPath() + ".pos", " \n");
        tokenizeSimple(document, parser, docFreq);
    }
}

void NgramTokenizer::tokenizeSimple(Document & document, Parser & parser,
        const std::shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < _nValue && parser.hasNext(); ++i)
        ngram.push_back(parser.next());

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(getMapping(wordified), 1, docFreq);
        ngram.pop_front();
        ngram.push_back(parser.next());
    }

    // add the last token
    document.increment(getMapping(wordify(ngram)), 1, docFreq);
}

void NgramTokenizer::tokenizeWord(Document & document,
        const std::shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    Parser parser(document.getPath() + ".sen", " \n");

    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < _nValue && parser.hasNext(); ++i)
    {
        string next = "";
        do
        {
            next = stopOrStem(parser.next());
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(getMapping(wordified), 1, docFreq);
        ngram.pop_front();
        string next = "";
        do
        {
            next = stopOrStem(parser.next());
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the last token
    document.increment(getMapping(wordify(ngram)), 1, docFreq);
}

string NgramTokenizer::stopOrStem(const string & str) const
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

void NgramTokenizer::tokenizeFW(Document & document,
        const std::shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    Parser parser(document.getPath() + ".sen", " \n");

    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < _nValue && parser.hasNext(); ++i)
    {
        string next = "";
        do
        {
            next = parser.next();
        } while(_functionWords.find(next) == _functionWords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(getMapping(wordified), 1, docFreq);
        ngram.pop_front();
        string next = "";
        do
        {
            next = parser.next();
        } while(_functionWords.find(next) == _functionWords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the last token
    document.increment(getMapping(wordify(ngram)), 1, docFreq);
}

void NgramTokenizer::initStopwords()
{
    auto config = io::config_reader::read("tokenizer.ini");
    Parser parser(config["stop-words"], "\n");
    while(parser.hasNext())
        _stopwords.insert(stemmers::Porter2Stemmer::stem(parser.next()));
}

void NgramTokenizer::initFunctionWords()
{
    auto config = io::config_reader::read("tokenizer.ini");
    Parser parser(config["function-words"], " \n");
    while(parser.hasNext())
        _functionWords.insert(parser.next());
}

size_t NgramTokenizer::getNValue() const
{
    return _nValue;
}

string NgramTokenizer::wordify(const deque<string> & words) const
{
    string result = "";
    for(auto & word: words)
        result += (word + " ");
    result = result.substr(0, result.size() - 1);
    return result;
}

string NgramTokenizer::setLower(const string & original) const
{
    string word = "";
    for(auto ch: original)
    {
        if(ch > 'A' && ch < 'Z')
            ch += 32;
    }
    return word;
}

}
}
