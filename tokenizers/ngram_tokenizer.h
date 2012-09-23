/**
 * @file ngram_tokenizer.h
 */

#ifndef _NGRAM_TOKENIZER_H_
#define _NGRAM_TOKENIZER_H_

#include <memory>
#include <deque>
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
         * Enumeration representing different ways to tokenize word tokens. 
         */
        enum NgramType { POS, Word };

        /**
         * Constructor.
         * @param n - the value of n to use for the ngrams.
         * @param type - indicates whether this tokenizer is tokenizing words or
         *  POS tags
         */
        NgramTokenizer(size_t n, NgramType type);

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        virtual void tokenize(Document & document,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreqs);

        /**
         * @return the value of n used for the ngrams
         */
        size_t getNValue() const;

    protected:

        /** Represents the extension for the ngram type */
        string _extension;

        /** the value of N in Ngram */
        size_t _nValue;

        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<std::string> _stopwords;
        
        /**
         * Turns a list of words into an ngram string.
         * @param words - the deque representing a list of words
         * @return the ngrams in string format
         */
        std::string wordify(const std::deque<std::string> & words) const;

        /**
         * Sets up a set of stopwords.
         */
        void initStopwords();

        /**
         * @param original - the string to set to lowercase
         * @return a lowercase version of the string
         */
        std::string setLower(const std::string & original) const;
};

#endif
