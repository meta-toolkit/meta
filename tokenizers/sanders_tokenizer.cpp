/**
 * @file sanders_tokenizer.cpp
 */

#include "sanders_tokenizer.h"

void SandersTokenizer::tokenize(const string & content, Document & document, unordered_map<TermID, unsigned int>* docFreq)
{
    struct sb_stemmer* stemmer = sb_stemmer_new("english", NULL);
    istringstream iss(content);
    vector<string> tokens;

    // insert all tokens into a vector
    copy(std::istream_iterator<string>(iss),
         std::istream_iterator<string>(),
         std::back_inserter<vector<string> >(tokens));

    // initialize the ngram
    vector<string> ngram;
    vector<string>::iterator it = tokens.begin();
    size_t i = 0;
    while(it != tokens.end() && i < _nValue)
    {
        string next = "";
        do
        {
            next = stem(*it, stemmer);
        } while(++it != tokens.end() && _stopwords.find(next) != _stopwords.end());
        ngram.push_back(next);
        ++i;
    }

    // add the rest of the ngrams
    while(it != tokens.end())
    {
        string wordified = wordify(ngram);
        document.increment(getMapping(wordified), 1, docFreq);
        ngram.erase(ngram.begin());
        string next = "";
        do
        {
            next = stem(*it, stemmer);
        } while(++it != tokens.end() && _stopwords.find(next) != _stopwords.end());
        ngram.push_back(next);
    }

    sb_stemmer_delete(stemmer);
}
