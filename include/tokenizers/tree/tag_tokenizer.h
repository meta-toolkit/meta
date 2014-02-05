/**
 * @file tag_tokenizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_TOKENIZERS_TAG_TOKENIZER_H_
#define _META_TOKENIZERS_TAG_TOKENIZER_H_

#include "tokenizers/tree/tree_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees by looking at labels of leaf and interior nodes.
 */
class tag_tokenizer : public tree_tokenizer<tag_tokenizer> {
    public:
        /**
         * Counts occurrences of leaf and interior node labels.
         * @param doc The document to parse
         * @param tree The current parse_tree in the document
         */
        void tree_tokenize(corpus::document & doc, const parse_tree & tree);
};

}
}

#endif
