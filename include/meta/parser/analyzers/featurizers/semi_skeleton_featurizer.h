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

#include "meta/parser/analyzers/featurizers/tree_featurizer.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by keeping track of only a single node label and
 * the underlying tree structure.
 * @see Sean Massung, ChengXiang Zhai, and Julia Hockenmaier. 2013. "Structural
 * Parse Tree Features for Text Representation"
 * @see http://web.engr.illinois.edu/~massung1/files/icsc-2013.pdf
 */
class semi_skeleton_featurizer
    : public util::clonable<tree_featurizer, semi_skeleton_featurizer>
{
  public:
    void tree_tokenize(const parser::parse_tree& tree,
                       featurizer& counts) const override;

    /// Identifier for this featurizer
    const static util::string_view id;
};
}
}
#endif
