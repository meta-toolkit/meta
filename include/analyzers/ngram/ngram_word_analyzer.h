/**
 * @file ngram_word_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_WORD_TOKENIZER_H_
#define _NGRAM_WORD_TOKENIZER_H_

#include "analyzers/ngram/ngram_analyzer.h"
#include "util/clonable.h"

namespace meta {
namespace analyzers {

class ngram_word_analyzer : public util::multilevel_clonable<
                                analyzer, ngram_analyzer, ngram_word_analyzer>
{
    using base = util::multilevel_clonable<analyzer, ngram_analyzer,
                                           ngram_word_analyzer>;
    public:
        /**
         * Constructor.
         * @param n The value of n to use for the ngrams.
         * @param stream The stream to read tokens from.
         */
        ngram_word_analyzer(uint16_t n, std::unique_ptr<token_stream> stream);

        /**
         * Copy constructor.
         */
        ngram_word_analyzer(const ngram_word_analyzer& other);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;

    private:
        /**
         * The token stream to be used for extracting tokens.
         */
        std::unique_ptr<token_stream> stream_;
};

}
}

#endif
