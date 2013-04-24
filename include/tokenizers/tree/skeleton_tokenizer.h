/**
 * @file skeleton_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_TOKENIZERS_SKELETON_TOKENIZER_H_
#define _META_TOKENIZERS_SKELETON_TOKENIZER_H_

#include "tokenizers/tree/tree_tokenizer.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees by only tokenizing the tree structure itself.
 */
class skeleton_tokenizer : public tree_tokenizer<skeleton_tokenizer> {
    public:
        using mapping_fn = std::function<term_id(const std::string &)>;
        using doc_freq_ptr = std::shared_ptr<std::unordered_map<term_id, unsigned int>>;

        /**
         * Ignores node labels and only tokenizes the tree structure.
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
