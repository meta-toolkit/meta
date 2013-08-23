/**
 * @file ngram_char_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_CHAR_TOKENIZER_H_
#define _NGRAM_CHAR_TOKENIZER_H_

#include "tokenizers/ngram/ngram_simple_tokenizer.h"

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
         * Tokenizes a file into a document.
         * @param document - the document to store the tokenized information in
         * @param mapping - the string to term_id mapping
         */
        virtual void tokenize_document(
                index::document & document,
                std::function<term_id(const std::string & term)> mapping);
};

}
}

#endif
