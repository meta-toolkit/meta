/**
 * @file tag_featurizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_TAG_FEATURIZER_H_
#define META_TAG_FEATURIZER_H_

#include "meta/parser/analyzers/featurizers/tree_featurizer.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by looking at labels of leaf and interior nodes.
 */
class tag_featurizer : public util::clonable<tree_featurizer, tag_featurizer>
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
