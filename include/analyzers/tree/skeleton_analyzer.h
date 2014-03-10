/**
 * @file skeleton_analyzer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SKELETON_ANALYZER_H_
#define META_SKELETON_ANALYZER_H_

#include "analyzers/tree/tree_analyzer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by only tokenizing the tree structure itself.
 */
class skeleton_analyzer
    : public util::multilevel_clonable<analyzer,
        tree_analyzer<skeleton_analyzer>, skeleton_analyzer>
{
  public:
    /**
     * Ignores node labels and only tokenizes the tree structure.
     * @param doc The document to parse
     * @param tree The current parse_tree in the document
     */
    void tree_tokenize(corpus::document& doc, const parse_tree& tree);

    /**
     * Identifier for this analyzer.
     */
    const static std::string id;
};
}
}

#endif
