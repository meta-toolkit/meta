/**
 * @file skeleton_featurizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SKELETON_FEATURIZER_H_
#define META_SKELETON_FEATURIZER_H_

#include "parser/analyzers/tree_analyzer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by only tokenizing the tree structure itself.
 */
class skeleton_featurizer
    : public util::clonable<tree_featurizer, skeleton_featurizer>
{
  public:
    /**
     * Ignores node labels and only tokenizes the tree structure.
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
