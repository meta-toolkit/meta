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

#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by only tokenizing the tree structure itself.
 * @see Sean Massung, ChengXiang Zhai, and Julia Hockenmaier. 2013. "Structural
 * Parse Tree Features for Text Representation"
 * @see http://web.engr.illinois.edu/~massung1/files/icsc-2013.pdf
 */
class skeleton_featurizer
    : public util::clonable<tree_featurizer, skeleton_featurizer>
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
