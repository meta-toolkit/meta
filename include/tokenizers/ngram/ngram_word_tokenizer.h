/**
 * @file ngram_word_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _NGRAM_WORD_TOKENIZER_H_
#define _NGRAM_WORD_TOKENIZER_H_

#include "stemmers/porter2.h"
#include "tokenizers/ngram/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Container for traits for the ngram_word_tokenizer. We need this in order
 * to have the trait parameters be the same, regardless of the outer
 * template.
 */
struct ngram_word_traits
{
    /**
     * Enumeration for which stemmer (if any) to use.
     */
    enum StopwordType { Default, NoStopwords };
};

template <class Stemmer = stemmers::porter2>
class ngram_word_tokenizer: public ngram_tokenizer, private Stemmer
{
    public:
        /**
         * Constructor.
         * @param n - the value of n to use for the ngrams.
         * @param type - indicates whether this tokenizer is tokenizing words or
         *  POS tags
         */
        ngram_word_tokenizer(
                size_t n,
                ngram_word_traits::StopwordType = ngram_word_traits::Default);

        /**
         * Tokenizes a file into a document.
         * @param document - the document to store the tokenized information in
         * @param mapping - the string to term_id mapping
         */
        virtual void tokenize_document(
                corpus::document & document,
                std::function<term_id(const std::string &)> mapping) override;

    private:

        /** The stemming function of the Stemmer policy class. */
        using Stemmer::stem;

        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<std::string> _stopwords;

        /**
         * Loads in a list of stopwords to this tokenizer.
         */
        void init_stopwords();
};

}
}

#include "tokenizers/ngram/ngram_word_tokenizer.tcc"
#endif
