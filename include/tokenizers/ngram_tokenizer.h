/**
 * @file ngram_tokenizer.h
 */

#ifndef _NGRAM_TOKENIZER_H_
#define _NGRAM_TOKENIZER_H_

#include <string>
#include <memory>
#include <deque>
#include <unordered_set>
#include <unordered_map>

#include "tokenizers/tokenizer.h"
#include "index/document.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes documents based on an ngram word model, where the value for n is supplied by the user.
 */
class NgramTokenizer : public Tokenizer
{
    public:
        /**
         * Enumeration representing different ways to tokenize word tokens. 
         */
        enum NgramType { POS, Word, FW, Char, Lex };

        /**
         * Enumeration for which stemmer (if any) to use.
         */
        enum StemmerType { Porter2, NoStemmer };

        /**
         * Enumeration for which stemmer (if any) to use.
         */
        enum StopwordType { Default, NoStopwords };

        /**
         * Constructor.
         * @param n - the value of n to use for the ngrams.
         * @param type - indicates whether this tokenizer is tokenizing words or
         *  POS tags
         */
        NgramTokenizer(size_t n, NgramType ngramType,
                StemmerType stemmerType = Porter2, StopwordType = Default);

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        virtual void tokenize(index::Document & document,
                const std::shared_ptr<std::unordered_map<index::TermID, unsigned int>> & docFreqs = nullptr);

        /**
         * @return the value of n used for the ngrams
         */
        size_t getNValue() const;

    private:

        /** the value of N in Ngram */
        size_t _nValue;

        /** shows how we're tokenizing text */
        NgramType _ngramType;

        /** shows how we're tokenizing text */
        StemmerType _stemmerType;

        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<std::string> _stopwords;
        
        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<std::string> _functionWords;

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
         * Sets up a set of function words.
         */
        void initFunctionWords();

        /**
         * Tokenizes text based on function word usage.
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        void tokenizeFW(index::Document & document,
            const std::shared_ptr<std::unordered_map<index::TermID, unsigned int>> & docFreqs);

        /**
         * Tokenizes text based on non-stopword tokens.
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        void tokenizeWord(index::Document & document,
            const std::shared_ptr<std::unordered_map<index::TermID, unsigned int>> & docFreqs);

        /**
         * Tokenizes text based on character ngrams.
         * @param document - the Document to store the tokenized information in
         * @param parser - the initialized document parser to use
         * @param docFreqs - optional parameter to store IDF values in
         */
        void tokenizeSimple(index::Document & document, io::Parser & parser,
            const std::shared_ptr<std::unordered_map<index::TermID, unsigned int>> & docFreqs);

        /**
         * @param original - the string to set to lowercase
         * @return a lowercase version of the string
         */
        std::string setLower(const std::string & original) const;

        /**
         * Removes stopwords or stems words depending on the options set in the
         * constructor. (Currently only stems words if that option is set)
         * @param str - the token to consider
         * @return the stemmed version of the word if the stemmer is enabled
         */
        std::string stopOrStem(const std::string & str) const;
};

}
}

#endif
