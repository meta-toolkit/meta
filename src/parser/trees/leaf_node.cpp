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

bool leaf_node::equal(const node& other) const
{
    return other.is_leaf() && category() == other.category()
           && word() == other.as<leaf_node>().word();
}

std::unique_ptr<node> leaf_node::accept(tree_transformer& trns) const
{
    return trns.transform(*this);
}
}
}
