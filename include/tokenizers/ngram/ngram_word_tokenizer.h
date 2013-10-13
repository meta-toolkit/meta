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

class ngram_word_tokenizer: public ngram_tokenizer
{
    public:
        enum class stopword_t {
            Default, None
        };

        /**
         * Constructor.
         * @param n - the value of n to use for the ngrams.
         * @param type - indicates whether this tokenizer is tokenizing words or
         *  POS tags
         */
        ngram_word_tokenizer(
                size_t n,
                stopword_t stopwords = stopword_t::Default,
                std::function<std::string(const std::string &)> stemmer =
                    stemmers::Porter2Stemmer::stem);

        /**
         * Tokenizes a file into a document.
         * @param document - the document to store the tokenized information in
         * @param mapping - the string to term_id mapping
         */
        virtual void tokenize_document(
                corpus::document & document,
                std::function<term_id(const std::string &)> mapping) override;

    private:

        /** The stemming function */
        std::function<std::string(const std::string &)> _stemmer;

        /**
         * A stopword list based on the stopwords list in the
         * configuration file
         * */
        std::unordered_set<std::string> _stopwords;

        /**
         * Loads in a list of stopwords to this tokenizer.
         */
        void init_stopwords();
};

}
}
#endif
