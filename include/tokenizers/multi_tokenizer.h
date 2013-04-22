/**
 * @file multi_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _MULTI_TOKENIZER_
#define _MULTI_TOKENIZER_

#include <memory>
#include <string>
#include <vector>
#include "tokenizers/tokenizer.h"
#include "index/document.h"

namespace meta {
namespace tokenizers {

/**
 * The multi_tokenizer class contains more than one tokenizer. This is useful for
 * trying combined feature methods.
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
         * @param document - the document to store the tokenized information in
         * @param mapping - the string to term_id mapping
         * @param docFreq - optional parameter to store IDF values in
         */
        void tokenize_document(index::document & document,
                std::function<term_id(const std::string &)> mapping,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq = nullptr);

    private:

        /** Holds all the tokenizers in this multi_tokenizer */
        std::vector<std::shared_ptr<tokenizer>> _tokenizers;
};

}
}

#endif
