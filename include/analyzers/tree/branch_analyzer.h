/**
 * @file branch_analyzer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_BRANCH_ANALYZER_H_
#define META_BRANCH_ANALYZER_H_

#include "analyzers/tree/tree_analyzer.h"
#include "util/clonable.h"

namespace meta {
namespace analyzers {

/**
 * Tokenizes parse trees by extracting branching factor features.
 */
class branch_analyzer
    : public util::multilevel_clonable<analyzer, tree_analyzer<branch_analyzer>,
                                       branch_analyzer>
{
    public:
        /**
         * Keeps track of the branching factor for this document's parse_trees.
         * @param doc The document to parse
         * @param tree The current parse_tree in the document
         */
        void tree_tokenize(corpus::document & doc, const parse_tree & tree);

        /**
         * Identifier for this analyzer.
         */
        const static std::string id;
};

}
}

#endif
