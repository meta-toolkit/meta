/**
 * @file ngram_fw_tokenizer.h
 */

#ifndef _NGRAM_FW_TOKENIZER_H_
#define _NGRAM_FW_TOKENIZER_H_

#include "tokenizers/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

class ngram_fw_tokenizer: public ngram_tokenizer
{
    public:

        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_fw_tokenizer(size_t n);

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

        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<std::string> _function_words;

        /**
         * Reads in specified function words from a file.
         */
        void init_function_words();
};

}
}

#endif
