/**
 * @file unary_chain_remover.cpp
 * @author Chase Geigle
 */

#include "meta/parser/trees/visitors/unary_chain_remover.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/util/shim.h"

namespace meta
{
namespace parser
{

std::unique_ptr<node> unary_chain_remover::operator()(const leaf_node& lnode)
{
    return make_unique<leaf_node>(lnode);
}

std::unique_ptr<node> unary_chain_remover::operator()(const internal_node& inode)
{
    // not a unary chain, so just invoke recursively on the children and
    // return the new tree
    if (inode.num_children() > 1)
    {
        auto ret = make_unique<internal_node>(inode.category());
        inode.each_child([&](const node* n)
                         {
                             ret->add_child(n->accept(*this));
                         });
        return std::move(ret);
    }

    // possible unary chain, so keep walking down it as long as the child
    // is the same label as the root
    const node* n = inode.child(0);
    while (inode.category() == n->category() && !n->is_leaf())
    {
        // if we have the same category but are no longer a unary
        // production, we can stop walking down the tree
        const auto& in = n->as<internal_node>();
        if (in.num_children() > 1)
            break;

        // otherwise, keep walking down the unary chain
        n = in.child(0);
    }

    if (n->is_leaf())
    {
        if (n->category() == inode.category())
            return make_unique<leaf_node>(n->as<leaf_node>());
        else
        {
            auto ret = make_unique<internal_node>(inode.category());
            ret->add_child(make_unique<leaf_node>(n->as<leaf_node>()));
            return std::move(ret);
        }
    }
    else
    {
        auto ret = make_unique<internal_node>(inode.category());
        const auto& in = n->as<internal_node>();
        // non-leaf, two cases for in:
        //
        //   (1) an internal node with a different label, in which case we
        //       recurse, then add this internal node as the child of the
        //       result and return
        //
        //   (2) an internal node with the same label, in which case we
        //       recurse on the children and graft them onto the result node
        //       and return
        if (inode.category() != in.category())
        {
            ret->add_child(in.accept(*this));
        }
        else
        {
            in.each_child([&](const node* child)
                          {
                              ret->add_child(child->accept(*this));
                          });
        }
        return std::move(ret);
    }
}
}
}
