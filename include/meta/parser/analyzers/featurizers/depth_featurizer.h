/**
 * @file depth_featurizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_DEPTH_FEATURIZER_H_
#define META_DEPTH_FEATURIZER_H_

#include "meta/parser/analyzers/featurizers/tree_featurizer.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by extracting depth features.
 */
template <class T>
class depth_featurizer
    : public util::clonable<tree_featurizer<T>, depth_featurizer<T>>
{
  public:
    using feature_map = typename depth_featurizer::feature_map;

    /**
     * Extracts the height of each parse tree.
     * @param tree The current parse_tree in the document
     * @param counts The feature_map to write to
     */
    void tree_tokenize(const parser::parse_tree& tree,
                       feature_map& counts) const override;

    /// Identifier for this featurizer
    const static util::string_view id;
};

// declare the valid instantiations for this featurizer
extern template class depth_featurizer<uint64_t>;
extern template class depth_featurizer<double>;
}
}
#endif
