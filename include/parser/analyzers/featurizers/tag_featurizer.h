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

#include "parser/analyzers/featurizers/tree_featurizer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by looking at labels of leaf and interior nodes.
 */
template <class T>
class tag_featurizer
    : public util::clonable<tree_featurizer<T>, tag_featurizer<T>>
{
  public:
    using feature_map = typename tag_featurizer::feature_map;

    /**
     * Counts occurrences of leaf and interior node labels.
     * @param tree The current parse_tree in the document
     * @param counts The feature_map to write to
     */
    void tree_tokenize(const parser::parse_tree& tree,
                       feature_map& counts) const override;

    /// Identifier for this featurizer
    const static std::string id;
};

extern template class tag_featurizer<uint64_t>;
extern template class tag_featurizer<double>;
}
}
#endif
