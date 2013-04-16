/**
 * @file ngram_simple_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_SIMPLE_TOKENIZER_H_
#define _NGRAM_SIMPLE_TOKENIZER_H_

#include "tokenizers/ngram_tokenizer.h"
#include "io/parser.h"

namespace meta {
namespace tokenizers {

/**
 * Derived classes from this simple ngram tokenizer differ only in file
 * extensions and Parsers used. Note this class is still abstract because
 * tokenize() is not defined.
 */
class ngram_simple_tokenizer: public ngram_tokenizer
{
    public:

        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_simple_tokenizer(size_t n);

    protected:

        /**
         * Tokenizes a file into a Document.
         * @param document The Document to store the tokenized information in
         * @param parser The parser to use for this document
         * @param docFreqs Optional parameter to store IDF values in
         */
        void simple_tokenize(
                io::Parser & parser,
                index::Document & document,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreqs = nullptr
        );
};

}
}

#endif
