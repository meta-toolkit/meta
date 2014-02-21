/**
 * @file ngram_char_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_CHAR_TOKENIZER_H_
#define _NGRAM_CHAR_TOKENIZER_H_

#include "analyzers/ngram/ngram_simple_analyzer.h"
#include "util/clonable.h"

namespace meta {
namespace analyzers {

class ngram_char_analyzer
    : public util::multilevel_clonable<analyzer, ngram_simple_analyzer,
                                       ngram_char_analyzer>
{
    using base = util::multilevel_clonable<analyzer, ngram_simple_analyzer,
                                           ngram_char_analyzer>;
    public:
        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_char_analyzer(uint16_t n);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & document) override;
};

}
}

#endif
