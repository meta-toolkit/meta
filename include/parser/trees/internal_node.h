/**
 * @file internal_node.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_INTERNAL_NODE_H_
#define META_PARSE_INTERNAL_NODE_H_

#include <memory>
#include <vector>
#include "parser/trees/node.h"

namespace meta
{
namespace parser
{

class leaf_node;

class internal_node : public node
{
  public:
    using node::node;

    /**
     * Constructs a new internal node by **moving** the children into
     * the node from a sequence denoted by iterators.
     *
     * @param cat The desired category for this node
     * @param begin An iterator to the beginning of a sequence of
     * children for this node
     * @param end An iterator to the ending of a sequence of children
     * for this node
     */
    template <class FwdIt>
    internal_node(class_label cat, FwdIt begin, FwdIt end)
        : node{std::move(cat)}
    {
        auto dist = std::distance(begin, end);
        children_.reserve(dist);
        for (; begin != end; ++begin)
            children_.emplace_back(std::move(*begin));
    }

    /**
     * Constructs a new internal node by moving the children into the node
     * from another vector of children.
     *
     * @param cat The desired category for this node
     * @param children The vector of children, which will be moved into
     * this node
     */
    internal_node(class_label cat,
                  std::vector<std::unique_ptr<node>>&& children);

    /**
     * Adds a child to this node.
     *
     * @param child The child to be added
     */
    void add_child(std::unique_ptr<node> child);

    /**
     * @return the number of children this node has
     */
    uint64_t num_children() const;

    /**
     * @param idx The index of the child to fetch
     * @return a non-owning pointer to the child at that index from
     * left to right
     */
    const node* child(uint64_t idx) const;

    bool is_leaf() const override;

    bool equal(const node& other) const override;

    const leaf_node* head_lexicon() const;

    void head_lexicon(const leaf_node* l);

    const node* head_constituent() const;

    void head_constituent(const node* n);

    template <class Fun>
    void each_child(Fun&& fn) const
    {
        for (const auto& child : children_)
            fn(child.get());
    }

    template <class Fun>
    void each_child(Fun&& fn)
    {
        for (auto& child : children_)
            fn(child.get());
    }

  private:
    /**
     * A list of the children of this node, from left to right
     */
    std::vector<std::unique_ptr<node>> children_;

    /**
     * A pointer to the head **word** for this subtree.
     */
    const leaf_node* head_lexicon_ = nullptr;

    /**
     * A pointer to the head constituent for this subtree (one of this
     * tree's children)
     */
    const node* head_constituent_ = nullptr;
};
}
}

#endif
