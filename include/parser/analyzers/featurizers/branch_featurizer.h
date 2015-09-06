/**
 * @file branch_featurizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_BRANCH_FEATURIZER_H_
#define META_BRANCH_FEATURIZER_H_

#include "parser/analyzers/featurizers/tree_featurizer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by extracting branching factor features.
 */
template <class T>
class branch_featurizer
    : public util::clonable<tree_featurizer<T>, branch_featurizer<T>>
{
  public:
    using feature_map = typename branch_featurizer::feature_map;

    /**
     * Keeps track of the branching factor for this document's parse_trees.
     * @param tree The current parse_tree in the document
     * @param counts The feature_map to write to
     */
    void tree_tokenize(const parser::parse_tree& tree,
                       feature_map& counts) const override;

    /// Identifier for this featurizer
    const static std::string id;
};

// declare the valid instantiations for this featurizer
extern template class branch_featurizer<uint64_t>;
extern template class branch_featurizer<double>;
}
}
#endif
