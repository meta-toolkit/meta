/**
 * @file tree_featurizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_TREE_FEATURIZER_H_
#define META_TREE_FEATURIZER_H_

#include "corpus/document.h"
#include "parser/trees/parse_tree.h"
#include "analyzers/analyzer.h"

namespace meta
{
namespace analyzers
{

/**
 * Base class for featurizers that convert trees into features in a
 * document.
 */
template <class T>
class tree_featurizer
{
  public:
    using feature_map = typename analyzer<T>::feature_map;
    using feature_value_type = T;
    using base_type = tree_featurizer;

    /**
     * Destructor.
     */
    virtual ~tree_featurizer() = default;

    /**
     * @param tree The parse tree, belonging to doc, to extract features
     * from
     * @param counts The feature_map to write to
     */
    virtual void tree_tokenize(const parser::parse_tree& tree,
                               feature_map& counts) const = 0;
};
}
}
#endif
