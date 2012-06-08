/**
 * @file ngram_tokenizer.h
 */

#ifndef _NGRAM_TOKENIZER_H_
#define _NGRAM_TOKENIZER_H_

#include <string.h>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "libstemmer/libstemmer.h"
#include "parser.h"
#include "document.h"
#include "parse_tree.h"
#include "tokenizer.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;

/**
 * Tokenizes documents based on an ngram word model, where the value for n is supplied by the user.
 */
class NgramTokenizer : public Tokenizer
{
    public:
        /**
         * Constructor.
         * @param n - the value of n to use for the ngrams.
         */
        NgramTokenizer(size_t n);

        /**
         *
         */
        void tokenize(const string & filename, Document & document, unordered_map<string, size_t>* docFreqs) const;

        /**
         * @return the value of n used for the ngrams
         */
        size_t getNValue() const;

    private:

        size_t _nValue;
        unordered_set<string> _stopwords;
        
        /**
         * Uses the Snowball stemmer.
         * @param word - the word to stem
         * @param stemmer - the stemmer to use
         * @return the stemmed version of the word
         */
        string stem(const string & word, struct sb_stemmer* stemmer) const;
    
        /**
         * Simply changes all letters to lowercase in a word.
         * @param word - the word to change
         * @return the value of n used for the ngrams
         */
        string setLower(const string & word) const;

        /**
         * Turns a list of words into an ngram string.
         * @param words - the vectoring representing a list of words
         * @return the ngrams in string format
         */
        string wordify(const vector<string> & words) const;

        /**
         * Sets up a set of stopwords.
         */
        void initStopwords();
};

#endif
