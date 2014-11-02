/**
 * @file internal_node.cpp
 * @author Chase Geigle
 */

#include "parser/tree/internal_node.h"

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

}
}
