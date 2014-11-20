/**
 * @file node.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_TREE_NODE_H_
#define META_PARSE_TREE_NODE_H_

#include <memory>
#include "meta.h"

namespace meta
{
namespace parser
{

class tree_transformer;

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
     * @param other The other subtree to compare with
     * @return whether this subtree is equal to the other subtree
     */
    virtual bool equal(const node& other) const = 0;

    /**
     * Accepts a transformer.
     * @param trns The transformer to be run on this subtree
     * @return the transformed root of the tree, or null if it was removed
     */
    virtual std::unique_ptr<node> accept(tree_transformer&) const = 0;

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
        return static_cast<Node&>(*this);
    }

    /**
     * Casting operation for nodes to derived types. Node can only be a
     * parser::leaf_node or parser::internal_node, and must be consistent
     * with is_leaf().
     *
     * @return this node cast to the given node type
     */
    template <class Node>
    typename std::add_const<Node>::type& as() const
    {
        return static_cast<typename std::add_const<Node>::type&>(*this);
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
