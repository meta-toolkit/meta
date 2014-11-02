/**
 * @file node.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_TREE_NODE_H_
#define META_PARSE_TREE_NODE_H_

#include "meta.h"

namespace meta
{
namespace parser
{

/**
 * A single node in a parse tree for a sentence.
 */
class node
{
  public:
    /**
     * Constructs a new node with the given category
     * @param cat The category for the new node
     */
    node(class_label cat);

    /**
     * @return the category for the node
     */
    const class_label& category() const;

    /**
     * @return whether this node is a leaf node
     */
    virtual bool is_leaf() const = 0;

    /**
     * Casting operation for nodes to derived types. Node can only be a
     * parser::leaf_node or parser::internal_node, and must be consistent
     * with is_leaf().
     *
     * @return this node cast to the given node type
     */
    template <class Node>
    Node& as()
    {
        return static_cast<Node>(*this);
    }

    /**
     * Casting operation for nodes to derived types. Node can only be a
     * parser::leaf_node or parser::internal_node, and must be consistent
     * with is_leaf().
     *
     * @return this node cast to the given node type
     */
    template <class Node>
    const Node& as() const
    {
        return static_cast<const Node>(*this);
    }

  private:
    /**
     * The category for this node
     */
    const class_label category_;
};
}
}
#endif
