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
template <class T>
class skeleton_featurizer
    : public util::clonable<tree_featurizer<T>, skeleton_featurizer<T>>
{
  public:
    using feature_map = typename skeleton_featurizer::feature_map;

    /**
     * Ignores node labels and only tokenizes the tree structure.
     * @param tree The current parse_tree in the document
     * @param counts The feature_map to write to
     */
    void tree_tokenize(const parser::parse_tree& tree,
                       feature_map& counts) const override;

    /// Identifier for this featurizer
    const static std::string id;
};

extern template class skeleton_featurizer<uint64_t>;
extern template class skeleton_featurizer<double>;
}
}
#endif
