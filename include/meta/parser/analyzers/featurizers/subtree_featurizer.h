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

#include "meta/parser/analyzers/featurizers/tree_featurizer.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by counting occurrences of subtrees in a
 * document's parse tree.
 */
template <class T>
class subtree_featurizer
    : public util::clonable<tree_featurizer<T>, subtree_featurizer<T>>
{
  public:
    using feature_map = typename subtree_featurizer::feature_map;

    /**
     * Counts occurrences of subtrees in this document's parse_trees.
     * @param tree The current parse_tree in the document
     * @param counts The feature_map to write to
     */
    void tree_tokenize(const parser::parse_tree& tree,
                       feature_map& counts) const override;

    /// Identifier for this featurizer
    const static util::string_view id;
};

// declare the valid instantiations for this featurizer
extern template class subtree_featurizer<uint64_t>;
extern template class subtree_featurizer<double>;
}
}
#endif
