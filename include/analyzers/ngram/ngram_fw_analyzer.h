/**
 * @file ngram_fw_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_FW_TOKENIZER_H_
#define _NGRAM_FW_TOKENIZER_H_

#include <unordered_set>
#include "analyzers/ngram/ngram_analyzer.h"
#include "util/clonable.h"

namespace meta {
namespace analyzers {

class ngram_fw_analyzer : public util::multilevel_clonable<
                              analyzer, ngram_analyzer, ngram_fw_analyzer>
{
    using base =
        util::multilevel_clonable<analyzer, ngram_analyzer, ngram_fw_analyzer>;
    public:
        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_fw_analyzer(uint16_t n);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;

    private:
        /** a stopword list based on the Lemur stopwords */
        std::unordered_set<std::string> _function_words;
};

}
}

#endif
