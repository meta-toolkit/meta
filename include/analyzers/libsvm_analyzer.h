/**
 * @file libsvm_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _LIBSVM_TOKENIZER_
#define _LIBSVM_TOKENIZER_

#include "analyzers/analyzer.h"
#include "util/clonable.h"

namespace meta {
namespace analyzers {

/**
 * libsvm_analyzer tokenizes documents that have been created from a
 * line_corpus, where each line is in libsvm input format and stored in the
 * document's content field.
 */
class libsvm_analyzer: public util::clonable<analyzer, libsvm_analyzer>
{
    public:
        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;

        /**
         * Identifier for this analyzer.
         */
        const static std::string id;
};

}
}

#endif
