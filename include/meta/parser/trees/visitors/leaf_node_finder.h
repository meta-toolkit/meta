/**
 * @file leaf_node_finder.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_LEAF_NODE_FINDER_H_
#define META_PARSER_LEAF_NODE_FINDER_H_

#include <memory>
#include <vector>

#include "meta/parser/trees/visitors/visitor.h"

namespace meta
{
namespace parser
{

/**
 * This is a visitor that finds all of the leaf nodes in a parse tree. The
 * list is built via side-effect, so the visiting methods return void and a
 * separate method is used to extract the leaf node list.
 *
 * The leaf nodes are copied into the list.
 */
class leaf_node_finder : public const_visitor<void>
{
  public:
    void operator()(const leaf_node&) override;
    void operator()(const internal_node&) override;

    /**
     * Returns the leaf nodes found by the visitor. This should be called
     * to extract the leaves after the visitor has been run. The leaves are
     * moved into the return value, so the visitor will be left empty by
     * this operation.
     */
    std::vector<std::unique_ptr<leaf_node>> leaves();

  private:
    /// The storage for the leaf nodes found so far
    std::vector<std::unique_ptr<leaf_node>> leaves_;
};


}
}

#endif
