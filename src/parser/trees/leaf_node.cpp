/**
 * @file leaf_node.cpp
 * @author Chase Geigle
 */

#include "parser/trees/leaf_node.h"
#include "parser/trees/transformers/tree_transformer.h"

namespace meta
{
namespace parser
{

leaf_node::leaf_node(class_label cat, std::string word)
    : node{std::move(cat)}, word_{std::move(word)}
{
    // nothing
}

const util::optional<std::string>& leaf_node::word() const
{
    return word_;
}

bool leaf_node::is_leaf() const
{
    return true;
}

std::unique_ptr<node> leaf_node::accept(tree_transformer& trns) const
{
    return trns.transform(*this);
}
}
}
