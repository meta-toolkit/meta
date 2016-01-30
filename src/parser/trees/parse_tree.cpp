/**
 * @file parse_tree.cpp
 */

#include <ostream>
#include "meta/parser/trees/parse_tree.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/visitors/tree_transformer.h"

namespace meta
{
namespace parser
{

namespace
{

// stolen from analyzers::parse_tree::pretty_print
void pretty_print(std::ostream& os, const node* n, uint64_t depth)
{
    std::string padding(depth, ' ');
    if (n->is_leaf())
    {
        const auto& leaf = n->as<leaf_node>();
        os << padding << "(" << leaf.category();
        if (auto word = leaf.word())
            os << " " << *leaf.word();
        os << ")\n";
    }
    else
    {
        const auto& inode = n->as<internal_node>();
        os << padding << "(" << inode.category() << "\n";
        inode.each_child([&](const node* child)
                         {
                             pretty_print(os, child, depth + 2);
                         });
        os << padding << ")\n";
    }
}

void print(std::ostream& os, const node* n)
{
    if (n->is_leaf())
    {
        const auto& leaf = n->as<leaf_node>();
        os << "(" << leaf.category();
        if (auto word = leaf.word())
            os << " " << *word;
        os << ")";
    }
    else
    {
        const auto& inode = n->as<internal_node>();
        os << "(" << inode.category();
        inode.each_child([&](const node* child)
                         {
                             os << " ";
                             print(os, child);
                         });
        os << ")";
    }
}
}

parse_tree::parse_tree(std::unique_ptr<node> root) : root_{std::move(root)}
{
    // nothing
}

parse_tree::parse_tree(const parse_tree& other) : root_{other.root_->clone()}
{
    // nothing
}

parse_tree& parse_tree::operator=(parse_tree rhs)
{
    swap(rhs);
    return *this;
}

void parse_tree::swap(parse_tree& other)
{
    std::swap(root_, other.root_);
}

void parse_tree::transform(tree_transformer& trns)
{
    root_ = root_->accept(trns);
}

void parse_tree::pretty_print(std::ostream& os) const
{
    ::meta::parser::pretty_print(os, root_.get(), 0);
}

std::ostream& operator<<(std::ostream& os, const parse_tree& tree)
{
    print(os, tree.root_.get());
    return os;
}

bool operator==(const parse_tree& lhs, const parse_tree& rhs)
{
    return lhs.root_->equal(*rhs.root_);
}
}
}
