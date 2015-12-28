/**
 * @file leaf_node.cpp
 * @author Chase Geigle
 */

#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

leaf_node::leaf_node(class_label cat, std::string word)
    : base{std::move(cat)}, word_{std::move(word)}
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
}
}
