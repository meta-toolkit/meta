/**
 * @file branch_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 * @author Chase Geigle
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
        using mapping_fn = std::function<term_id(const std::string &)>;

        /**
         * Keeps track of the branching factor for this document's parse_trees.
         * @param document - the document to parse
         * @param tree - the current parse_tree in the document
         */
        void tree_tokenize(corpus::document & document,
                           const parse_tree & tree, 
                           mapping_fn mapping);
};

}
}

#endif
