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
template <class T>
class semi_skeleton_featurizer
    : public util::clonable<tree_featurizer<T>, semi_skeleton_featurizer<T>>
{
  public:
    using feature_map = typename semi_skeleton_featurizer::feature_map;

    /**
     * Keeps track of one node's tag and the skeleton structure beneath it.
     * @param tree The current parse_tree in the document
     * @param counts The feature_map to write to
     */
    void tree_tokenize(const parser::parse_tree& tree,
                       feature_map& counts) const override;

    /// Identifier for this featurizer
    const static std::string id;
};

extern template class semi_skeleton_featurizer<uint64_t>;
extern template class semi_skeleton_featurizer<double>;
}
}
#endif
