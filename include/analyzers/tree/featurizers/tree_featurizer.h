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
    /**
     * Destructor.
     */
    virtual ~tree_featurizer() = default;

    /**
     * @param doc The document to add feature counts to
     * @param tree The parse tree, belonging to doc, to extract features
     * from
     */
    virtual void tree_tokenize(corpus::document& doc,
                               const parser::parse_tree& tree) const = 0;
};
}
}

#endif
