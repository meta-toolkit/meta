/**
 * @file ngram_word_tokenizer.h
 */

#ifndef _NGRAM_WORD_TOKENIZER_H_
#define _NGRAM_WORD_TOKENIZER_H_

#include "tokenizers/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

class ngram_word_tokenizer: public ngram_tokenizer
{
    public:

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
        ngram_word_tokenizer(size_t n, StemmerType stemmerType = Porter2,
                StopwordType = Default);

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        virtual void tokenize(
                index::Document & document,
                const std::shared_ptr<std::unordered_map<index::TermID, unsigned int>> & docFreqs = nullptr
        );

    private:

        /** shows how we're tokenizing text */
        StemmerType _stemmerType;

        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<std::string> _stopwords;

        /**
         * Loads in a list of stopwords to this tokenizer.
         */
        void init_stopwords();

        /**
         * Removes stopwords or stems words depending on the options set in the
         * constructor. (Currently only stems words if that option is set)
         * @param str - the token to consider
         * @return the stemmed version of the word if the stemmer is enabled
         */
        std::string stop_or_stem(const std::string & str) const;

};

}
}

#endif
