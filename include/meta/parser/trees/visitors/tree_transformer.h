/**
 * @file tree_transformer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_TREE_TRANSFORMER_H_
#define META_PARSE_TREE_TRANSFORMER_H_

#include <memory>
#include "meta/parser/trees/visitors/visitor.h"

namespace meta
{
namespace parser
{

class node;

/**
 * Abstract base class for tree transformers.
 */
class tree_transformer : public const_visitor<std::unique_ptr<node>>
{
  public:
    virtual std::unique_ptr<node> operator()(const leaf_node&) = 0;
    virtual std::unique_ptr<node> operator()(const internal_node&) = 0;
};
}
}
#endif
