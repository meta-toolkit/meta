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
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

/**
 * Abstract base class for tree transformers.
 */
class tree_transformer
{
  public:
    virtual std::unique_ptr<node> transform(const leaf_node&) = 0;
    virtual std::unique_ptr<node> transform(const internal_node&) = 0;
};
}
}
#endif
