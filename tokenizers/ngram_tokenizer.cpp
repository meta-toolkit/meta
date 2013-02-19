/**
 * @file ngram_tokenizer.cpp
 */

#include <string.h>
#include <cstdlib>
#include "util/common.h"
#include "stemmers/porter2_stemmer.h"
#include "index/document.h"
#include "io/parser.h"
#include "parse_tree.h"
#include "ngram_tokenizer.h"

using std::deque;
using std::unordered_map;
using std::unordered_set;

NgramTokenizer::NgramTokenizer(size_t n, NgramType type):
    _nValue(n),
    _type(type),
    _stopwords(unordered_set<string>()), 
    _functionWords(unordered_set<string>())
{
   if(type == Word)
       initStopwords();
   else if(type == FW)
       initFunctionWords();
}

void NgramTokenizer::tokenize(Document & document,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    if(_type == FW)
        tokenizeFW(document, docFreq);
    else if(_type == Word)
        tokenizeWord(document, docFreq);
    else if(_type == Char)
        tokenizeChar(document, docFreq);
    else // _type == POS
        tokenizePOS(document, docFreq);
}

void NgramTokenizer::tokenizePOS(Document & document,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    Parser parser(document.getPath() + ".pos", " \n");

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
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    Parser parser(document.getPath() + ".sen", " \n");

    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < _nValue && parser.hasNext(); ++i)
    {
        string next = "a"; // start with stopword
        while(_stopwords.find(next) != _stopwords.end() && parser.hasNext())
            next = Porter2Stemmer::stem(Porter2Stemmer::trim(parser.next()));
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(getMapping(wordified), 1, docFreq);
        ngram.pop_front();
        string next = "a";
        while(_stopwords.find(next) != _stopwords.end() && parser.hasNext())
            next = Porter2Stemmer::stem(Porter2Stemmer::trim(parser.next()));
        ngram.push_back(next);
    }

    // add the last token
    document.increment(getMapping(wordify(ngram)), 1, docFreq);
}

void NgramTokenizer::tokenizeChar(Document & document,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    Parser parser(document.getPath() + ".sen", "");

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

void NgramTokenizer::tokenizeFW(Document & document,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
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
    Parser parser("../data/lemur-stopwords.txt", "\n"); // TODO
    while(parser.hasNext())
        _stopwords.insert(Porter2Stemmer::stem(parser.next()));
}

void NgramTokenizer::initFunctionWords()
{
    Parser parser("../data/function-words.txt", " \n"); // TODO
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
