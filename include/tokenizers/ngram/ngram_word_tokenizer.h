/**
 * @file ngram_word_tokenizer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_WORD_TOKENIZER_H_
#define _NGRAM_WORD_TOKENIZER_H_

#include <functional>
#include "stemmers/porter2.h"
#include "tokenizers/ngram/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

class ngram_word_tokenizer: public ngram_tokenizer
{
    public:
        /** Signifies whether or not stopwords should be removed from the doc */
        enum class stopword_t { Default, None };

        /**
         * Constructor.
         * @param n The value of n to use for the ngrams.
         * @param type Indicates whether this tokenizer is tokenizing words, POS
         * tags, etc.
         * @param stemmer What stemming function (if any) to use for this
         * tokenizer
         */
        ngram_word_tokenizer(uint16_t n,
            stopword_t stopwords = stopword_t::Default,
            std::function<std::string(const std::string &)> stemmer =
                    stemmers::porter2{}
        );

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;

    private:
        /** The stemming function */
        std::function<std::string(const std::string &)> _stemmer;

        /**
         * A stopword list based on the stopwords list in the configuration file
         */
        std::unordered_set<std::string> _stopwords;

        /**
         * Loads in a list of stopwords to this tokenizer.
         */
        void init_stopwords();
};

}
}

#endif
