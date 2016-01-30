/**
 * @file tree_featurizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_TREE_FEATURIZER_H_
#define META_TREE_FEATURIZER_H_

#include "meta/corpus/document.h"
#include "meta/parser/trees/parse_tree.h"
#include "meta/analyzers/analyzer.h"

namespace meta
{
namespace analyzers
{

/**
 * Base class for featurizers that convert trees into features in a
 * document.
 */
class tree_featurizer
{
  public:
    using base_type = tree_featurizer;

    /**
     * Destructor.
     */
    virtual ~tree_featurizer() = default;

    /**
     * @param tree The parse tree, belonging to doc, to extract features
     * from
     * @param counts The featurizer to write to
     */
    virtual void tree_tokenize(const parser::parse_tree& tree,
                               featurizer& counts) const = 0;
};
}
}
#endif
