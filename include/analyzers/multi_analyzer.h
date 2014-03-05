/**
 * @file multi_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _MULTI_TOKENIZER_
#define _MULTI_TOKENIZER_

#include <vector>
#include <memory>

#include "analyzers/analyzer.h"
#include "util/clonable.h"

namespace meta {
namespace analyzers {

/**
 * The multi_analyzer class contains more than one analyzer. This is useful
 * for trying combined feature methods.
 *
 * For example, you could tokenize based on ngrams of words and parse tree
 * rewrite rules. The multi_analyzer keeps track of all the features in one set
 * for however many internal analyzers it contains.
 */
class multi_analyzer: public util::clonable<analyzer, multi_analyzer>
{
    public:
        /**
         * Constructs a multi_analyzer from a vector of other analyzers.
         * @param toks
         */
        multi_analyzer(std::vector<std::unique_ptr<analyzer>>&& toks);

        /**
         * Copy constructor.
         */
        multi_analyzer(const multi_analyzer& other);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;

    private:

        /** Holds all the analyzers in this multi_analyzer */
        std::vector<std::unique_ptr<analyzer>> _analyzers;
};

}
}

#endif
