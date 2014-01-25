/**
 * @file ngram_simple_tokenizer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_SIMPLE_TOKENIZER_H_
#define _NGRAM_SIMPLE_TOKENIZER_H_

#include "tokenizers/ngram/ngram_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Derived classes from this simple ngram tokenizer differ only in file
 * extensions and parsers used. Note this class is still abstract because
 * tokenize() is not defined.
 */
class ngram_simple_tokenizer: public ngram_tokenizer
{
    public:
        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_simple_tokenizer(uint16_t n);

    protected:
        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         * @param parser The parser to use for this document
         */
        void simple_tokenize(io::parser & parser, corpus::document & doc);
};

}
}

#endif
