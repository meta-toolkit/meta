/**
 * @file semi_skeleton_tokenizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_TOKENIZERS_SEMI_SKELETON_TOKENIZER_H_
#define _META_TOKENIZERS_SEMI_SKELETON_TOKENIZER_H_

#include "tokenizers/tree/tree_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees by keeping track of only a single node label and
 * the underlying tree structure.
 */
class semi_skeleton_tokenizer : public tree_tokenizer<semi_skeleton_tokenizer> {
    public:
        /**
         * Keeps track of one node's tag and the skeleton structure beneath it.
         * @param doc The document to parse
         * @param tree The current parse_tree in the document
         */
        void tree_tokenize(corpus::document & doc, const parse_tree & tree);
};

}
}

#endif
