/**
 * @file ngram_lex_tokenizer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_LEX_TOKENIZER_H_
#define _NGRAM_LEX_TOKENIZER_H_

#include "tokenizers/ngram/ngram_simple_tokenizer.h"

namespace meta {
namespace tokenizers {

class ngram_lex_tokenizer: public ngram_simple_tokenizer
{
    public:
        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_lex_tokenizer(uint16_t n);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;
};

}
}

#endif
