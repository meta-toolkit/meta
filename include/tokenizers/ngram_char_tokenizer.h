/**
 * @file ngram_char_tokenizer.h
 */

#ifndef _NGRAM_CHAR_TOKENIZER_H_
#define _NGRAM_CHAR_TOKENIZER_H_

#include "tokenizers/ngram_simple_tokenizer.h"

namespace meta {
namespace tokenizers {

class ngram_char_tokenizer: public ngram_simple_tokenizer
{
    public:

        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_char_tokenizer(size_t n);

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        virtual void tokenize(
                index::Document & document,
                const std::shared_ptr<std::unordered_map<TermID, unsigned int>> & docFreqs = nullptr
        );
};

}
}

#endif
