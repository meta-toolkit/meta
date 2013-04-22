/**
 * @file depth_tokenizer.h
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
        using mapping_fn = std::function<term_id(const std::string &)>;
        using doc_freq_ptr = std::shared_ptr<std::unordered_map<term_id, unsigned int>>;

        /**
         * Extracts the depth feature from parse trees: what are the heights of
         * a document's trees?
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this tokenizer
         */
        void tree_tokenize( index::document & document, const ParseTree & tree, 
                            mapping_fn mapping, const doc_freq_ptr & doc_freq );
};

}
}

#endif
