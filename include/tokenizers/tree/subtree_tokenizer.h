/**
 * @file subtree_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_TOKENIZERS_SUBTREE_TOKENIZER_H_
#define _META_TOKENIZERS_SUBTREE_TOKENIZER_H_

#include "tokenizers/tree/tree_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees by counting occurrences of subtrees in a
 * document's parse tree.
 */
class subtree_tokenizer: public tree_tokenizer<subtree_tokenizer> {
    public:
        using mapping_fn = std::function<term_id(const std::string &)>;
        using doc_freq_ptr = std::shared_ptr<std::unordered_map<term_id, unsigned int>>;

        /**
         * Counts occurrences of subtrees in this document's parse_trees.
         * @param document - the document to parse
         * @param tree - the current parse_tree in the document
         * @param docFreq - used to aggregate counts for this tokenizer
         */
        void tree_tokenize( index::document & document, const parse_tree & tree, 
                            mapping_fn mapping, const doc_freq_ptr & doc_freq );
};

}
}

#endif
