/**
 * @file branch_tokenizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_TOKENIZERS_BRANCH_TOKENIZER_H_
#define _META_TOKENIZERS_BRANCH_TOKENIZER_H_

#include "tokenizers/tree/tree_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees by extracting branching factor features.
 */
class branch_tokenizer: public tree_tokenizer<branch_tokenizer> {
    public:
        /**
         * Keeps track of the branching factor for this document's parse_trees.
         * @param doc The document to parse
         * @param tree The current parse_tree in the document
         */
        void tree_tokenize(corpus::document & doc, const parse_tree & tree);
};

}
}

#endif
