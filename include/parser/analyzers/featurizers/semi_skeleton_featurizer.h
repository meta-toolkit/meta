/**
 * @file semi_skeleton_featurizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SEMI_SKELETON_FEATURIZER_H_
#define META_SEMI_SKELETON_FEATURIZER_H_

#include "parser/analyzers/featurizers/tree_featurizer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by keeping track of only a single node label and
 * the underlying tree structure.
 */
class semi_skeleton_featurizer
    : public util::clonable<tree_featurizer, semi_skeleton_featurizer>
{
  public:
    /**
     * Keeps track of one node's tag and the skeleton structure beneath it.
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
