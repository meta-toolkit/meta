/**
 * @file annotation_remover.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_ANNOTATION_REMOVER_H_
#define META_ANNOTATION_REMOVER_H_

#include "meta/parser/trees/visitors/tree_transformer.h"

namespace meta
{
namespace parser
{

/**
 * A tree transformer that removes annotations (currently only Penn
 * Treebank style) from trees.
 *
 * This effectively truncates rule names before the first -, =, or |, with
 * a few exceptions (-LRB-, -RRB-, -, etc.).
 */
class annotation_remover : public tree_transformer
{
  public:
    std::unique_ptr<node> operator()(const leaf_node&) override;
    std::unique_ptr<node> operator()(const internal_node&) override;
};
}
}

#endif
