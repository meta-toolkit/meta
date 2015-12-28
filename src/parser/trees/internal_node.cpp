/**
 * @file internal_node.cpp
 * @author Chase Geigle
 */

#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

internal_node::internal_node(class_label cat,
                             std::vector<std::unique_ptr<node>>&& children)
    : base{std::move(cat)}, children_{std::move(children)}
{
    // nothing
}

internal_node::internal_node(const internal_node& other)
    : base{other.category()}
{
    other.each_child([&](const node* n)
                     {
                         children_.emplace_back(n->clone());
                         if (n == other.head_constituent())
                             head(children_.back().get());
                     });
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

void internal_node::head(const node* n)
{
#if DEBUG
    bool found = false;
    for (const auto& child : children_)
        found = found || (child.get() == n);
    if (!found)
        throw std::runtime_error{"Cannot set head to a non-child node"};
#endif
    head_constituent(n);

    if (n->is_leaf())
        head_lexicon(&n->as<leaf_node>());
    else
        head_lexicon(n->as<internal_node>().head_lexicon());
}
}
}
