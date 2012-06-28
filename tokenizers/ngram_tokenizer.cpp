/**
 * @file ngram_tokenizer.cpp
 */

#include "ngram_tokenizer.h"

NgramTokenizer::NgramTokenizer(size_t n):
    _nValue(n), _stopwords(unordered_set<string>())
{
   initStopwords();     
}

void NgramTokenizer::tokenize(const string & filename, Document & document, unordered_map<TermID, unsigned int>* docFreq)
{
    struct sb_stemmer* stemmer = sb_stemmer_new("english", NULL);
    string validchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'-";
    Parser parser(filename, validchars, validchars, validchars);

    // initialize the ngram
    vector<string> ngram;
    for(size_t i = 0; i < _nValue && parser.hasNext(); ++i)
    {
        string next = "";
        do
        {
            next = stem(parser.next(), stemmer);
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext() && i < _nValue);
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(getMapping(wordified), 1, docFreq);
        ngram.erase(ngram.begin());
        string next = "";
        do
        {
            next = stem(parser.next(), stemmer);
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    sb_stemmer_delete(stemmer);
}

void NgramTokenizer::initStopwords()
{
    struct sb_stemmer* stemmer = sb_stemmer_new("english", NULL);
    string valid = "abcdefghijklmnopqrstuvwxyzI";
    Parser parser("data/lemur-stopwords.txt", valid, valid, valid);
    while(parser.hasNext())
    {
        _stopwords.insert(stem(parser.next(), stemmer));
    }
    sb_stemmer_delete(stemmer);
}

size_t NgramTokenizer::getNValue() const
{
    return _nValue;
}

string NgramTokenizer::wordify(const vector<string> & words) const
{
    string result = "";
    for(auto & word: words)
        result += (word + " ");
    return result;
}

string NgramTokenizer::setLower(const string & word) const
{
    string lower = "";
    for(auto & ch: word)
        lower += tolower(ch);
    return lower;
}

string NgramTokenizer::stem(const string & word, struct sb_stemmer* stemmer) const
{
    size_t length = word.size();
    sb_symbol symb[length];
    const char* cstr = (setLower(word)).c_str();
    memcpy(symb, cstr, length);
    return string((char*)sb_stemmer_stem(stemmer, symb, length));
}
