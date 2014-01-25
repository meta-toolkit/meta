/**
 * @file multi_tokenizer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _MULTI_TOKENIZER_
#define _MULTI_TOKENIZER_

#include <vector>
#include <memory>

#include "tokenizers/tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * The multi_tokenizer class contains more than one tokenizer. This is useful
 * for trying combined feature methods.
 *
 * For example, you could tokenize based on ngrams of words and parse tree
 * rewrite rules. The multi_tokenizer keeps track of all the features in one set
 * for however many internal tokenizers it contains.
 */
class multi_tokenizer: public tokenizer
{
    public:
        /**
         * Constructs a multi_tokenizer from a vector of other tokenizers.
         * @param toks
         */
        multi_tokenizer(const std::vector<std::shared_ptr<tokenizer>> & toks);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;

    private:

        /** Holds all the tokenizers in this multi_tokenizer */
        std::vector<std::shared_ptr<tokenizer>> _tokenizers;
};

}
}

#endif
