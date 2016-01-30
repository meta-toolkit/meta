/**
 * @file unary_chain_remover.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_UNARY_CHAIN_REMOVER_H_
#define META_PARSE_UNARY_CHAIN_REMOVER_H_

#include "meta/parser/trees/visitors/tree_transformer.h"

namespace meta
{
namespace parser
{

/**
 * Transforms trees by removing any unary X -> X rules. These may arise
 * from filtering out trace/empty nodes, for example, and may cause
 * problems in parsing if they persist.
 */
class unary_chain_remover : public tree_transformer
{
  public:
    std::unique_ptr<node> operator()(const leaf_node&) override;
    std::unique_ptr<node> operator()(const internal_node&) override;
};
}
}

#endif
