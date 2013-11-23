/**
 * @file depth_tokenizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_TOKENIZERS_DEPTH_TOKENIZER_H_
#define _META_TOKENIZERS_DEPTH_TOKENIZER_H_

#include "tokenizers/tree/tree_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees by extracting depth features.
 */
class depth_tokenizer : public tree_tokenizer<depth_tokenizer> {
    public:
        /**
         * Extracts the height of each parse tree.
         * @param doc The document to parse
         * @param tree The current parse_tree in the document
         */
        void tree_tokenize(corpus::document & doc, const parse_tree & tree);
};

}
}

#endif
