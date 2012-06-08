/**
 * @file ngram_tokenizer.cpp
 */

#include "ngram_tokenizer.h"

void NgramTokenizer::tokenize(const string & filename, Document & document, unordered_map<string, size_t>* docFreq) const
{
    struct sb_stemmer* stemmer = sb_stemmer_new("english", NULL);
    string validchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'-";
    Parser parser(filename, validchars, validchars, validchars);

    // initialize the ngram
    vector<string> ngram;
    for(size_t i = 0; i < _nValue && parser.hasNext(); ++i)
    {
        string next = stem(parser.next(), stemmer);
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(wordified, 1, docFreq);
        ngram.erase(ngram.begin());
        string next = stem(parser.next(), stemmer);
        ngram.push_back(next);
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
    vector<string>::const_iterator iter;
    for(iter = words.begin(); iter != words.end(); ++iter)
        result += (*iter + " ");
    return result;
}

string NgramTokenizer::setLower(const string & word) const
{
    string lower = "";
    for(size_t i = 0; i < word.size(); ++i)
        lower += tolower(word[i]);
    return lower;
}

string NgramTokenizer::stem(const string & word, struct sb_stemmer* stemmer) const
{
    size_t length = word.size();
    sb_symbol symb[length];
    const char* cstr = (setLower(word)).c_str();
    memcpy(symb, cstr, length);
    return string((char*)sb_stemmer_stem(stemmer, symb, length));
    //return word;
}
