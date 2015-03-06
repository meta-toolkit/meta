/**
 * @file subtree_featurizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SUBTREE_FEATURIZER_H_
#define META_SUBTREE_FEATURIZER_H_

#include "parser/analyzers/featurizers/tree_featurizer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by counting occurrences of subtrees in a
 * document's parse tree.
 */
class subtree_featurizer
    : public util::clonable<tree_featurizer, subtree_featurizer>
{
  public:
    /**
     * Counts occurrences of subtrees in this document's parse_trees.
     * @param doc The document to parse
     * @param tree The current parse_tree in the document
     */
    void tree_tokenize(corpus::document& doc,
                       const parser::parse_tree& tree) const override;

    /// Identifier for this featurizer
    const static std::string id;
};
}
}

#endif
