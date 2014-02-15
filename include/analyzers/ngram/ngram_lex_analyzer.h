/**
 * @file ngram_lex_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_LEX_TOKENIZER_H_
#define _NGRAM_LEX_TOKENIZER_H_

#include "analyzers/ngram/ngram_simple_analyzer.h"

namespace meta {
namespace analyzers {

class ngram_lex_analyzer: public ngram_simple_analyzer
{
    public:
        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_lex_analyzer(uint16_t n);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;
};

}
}

#endif
