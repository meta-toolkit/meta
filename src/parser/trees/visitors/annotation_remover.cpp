/**
 * @file annotation_remover.cpp
 * @author Chase Geigle
 */

#include "meta/parser/trees/visitors/annotation_remover.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/util/shim.h"

namespace meta
{
namespace parser
{

namespace
{

/**
 * Normalizes a category label by removing any Penn Treebank style
 * annotations. The truncation occurs after the first character, in case
 * any of -, =, or | is a valid rule label.
 */
std::string normalize(const std::string& cat)
{
    auto dash = cat.find('-', 1);
    auto eq = cat.find('=', 1);
    auto pipe = cat.find('|', 1);
    if (dash != cat.npos && dash < eq && dash < pipe)
        return cat.substr(0, dash);
    if (eq != cat.npos && eq < dash && eq < pipe)
        return cat.substr(0, eq);
    if (pipe != cat.npos && pipe < dash && pipe < eq)
        return cat.substr(0, pipe);
    return cat;
}
}

std::unique_ptr<node> annotation_remover::operator()(const leaf_node& lnode)
{
    // we leave leaf nodes as-is: this avoids special casing things like
    // -LRB-, etc.
    return make_unique<leaf_node>(lnode);
}

std::unique_ptr<node> annotation_remover::operator()(const internal_node& inode)
{
    auto norm = normalize(inode.category());
    auto ret = make_unique<internal_node>(class_label{std::move(norm)});
    // first, normalize children
    inode.each_child([&](const node* child)
                     {
                         // no null check: this transform doesn't remove
                         // nodes
                         ret->add_child(child->accept(*this));
                     });
    return std::move(ret);
}
}
}
