/**
 * @file internal_node.cpp
 * @author Chase Geigle
 */

#include "parser/trees/internal_node.h"
#include "parser/trees/transformers/tree_transformer.h"

namespace meta
{
namespace parser
{

internal_node::internal_node(class_label cat,
                             std::vector<std::unique_ptr<node>>&& children)
    : node{std::move(cat)}, children_{std::move(children)}
{
    // nothing
}

void internal_node::add_child(std::unique_ptr<node> child)
{
    children_.emplace_back(std::move(child));
}

const node* internal_node::child(uint64_t idx) const
{
    return children_.at(idx).get();
}

uint64_t internal_node::num_children() const
{
    return children_.size();
}

bool internal_node::is_leaf() const
{
    return false;
}

bool internal_node::equal(const node& other) const
{
    if (other.is_leaf() || category() != other.category())
        return false;

    const auto& internal = other.as<internal_node>();

    if (num_children() != internal.num_children())
        return false;

    bool ret = true;
    for (size_t i = 0; i < num_children(); ++i)
        ret &= children_[i]->equal(*internal.children_[i]);
    return ret;
}

const leaf_node* internal_node::head_lexicon() const
{
    return head_lexicon_;
}

void internal_node::head_lexicon(const leaf_node* l)
{
    head_lexicon_ = l;
}

const node* internal_node::head_constituent() const
{
    return head_constituent_;
}

void internal_node::head_constituent(const node* n)
{
    head_constituent_ = n;
}

std::unique_ptr<node> internal_node::accept(tree_transformer& trns) const
{
    return trns.transform(*this);
}
}
}
