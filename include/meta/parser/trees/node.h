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
#include "meta/meta.h"
#include "meta/parser/trees/visitors/visitor.h"

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
     * @return whether this node is a temporary node inserted during
     * binarization
     */
    bool is_temporary() const;

    /**
     * @param other The other subtree to compare with
     * @return whether this subtree is equal to the other subtree
     */
    virtual bool equal(const node& other) const = 0;

    /**
     * Clones the given node.
     * @return a unique_ptr to a copy of this object
     */
    virtual std::unique_ptr<node> clone() const = 0;

    /**
     * Accepts a visitor.
     * @param vtor The visitor to visit each of the nodes in this subtree
     * @return the visitor result for this subtree
     */
    template <class Visitor>
    typename std::remove_reference<Visitor>::type::result_type
        accept(Visitor&& vtor)
    {
        // this is ugly, but since we cannot have virtual template member
        // functions, it's a necessary evil
        if (is_leaf())
            return vtor(as<leaf_node>());
        return vtor(as<internal_node>());
    }

    /**
     * Accepts a visitor (const version). Can only accept visitors that do
     * not modify the tree directly.
     *
     * @param vtor The visitor to visit each of the nodes in this subtree
     * @return the visitor result for this subtree
     */
    template <class Visitor>
    typename std::remove_reference<Visitor>::type::result_type
        accept(Visitor&& vtor) const
    {
        if (is_leaf())
            return vtor(as<leaf_node>());
        return vtor(as<internal_node>());
    }

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

    /**
     * Default virtual destructor for deriving classes.
     */
    virtual ~node() = default;

  private:
    /**
     * The category for this node
     */
    const class_label category_;
};
}
}
#endif
