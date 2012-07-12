/**
 * @file ngram_tokenizer.h
 */

#ifndef _NGRAM_TOKENIZER_H_
#define _NGRAM_TOKENIZER_H_

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "tokenizer.h"

class Document;

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
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        virtual void tokenize(Document & document, std::unordered_map<TermID, unsigned int>* docFreqs);

        /**
         * @return the value of n used for the ngrams
         */
        size_t getNValue() const;

    protected:

        /** the value of N in Ngram */
        size_t _nValue;

        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<string> _stopwords;
        
        /**
         * Uses the Snowball stemmer.
         * @param word - the word to stem
         * @param stemmer - the stemmer to use
         * @return the stemmed version of the word
         */
        std::string stem(const std::string & word, struct sb_stemmer* stemmer) const;
     
        /**
         * Turns a list of words into an ngram string.
         * @param words - the vectoring representing a list of words
         * @return the ngrams in string format
         */
        std::string wordify(const std::vector<std::string> & words) const;

        /**
         * Sets up a set of stopwords.
         */
        void initStopwords();
};

#endif
