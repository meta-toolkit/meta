/**
 * @file depth_analyzer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_TOKENIZERS_DEPTH_TOKENIZER_H_
#define _META_TOKENIZERS_DEPTH_TOKENIZER_H_

#include "analyzers/tree/tree_analyzer.h"

namespace meta {
namespace analyzers {

/**
 * Tokenizes parse trees by extracting depth features.
 */
class depth_analyzer : public tree_analyzer<depth_analyzer> {
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
