/**
 * @file parse_tree.cpp
 */

#include <ostream>
#include "parser/tree/parse_tree.h"
#include "parser/tree/leaf_node.h"
#include "parser/tree/internal_node.h"

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
        os << padding << "(\n" << padding << "  " << inode.category() << "\n";
        inode.each_child([&](const node* child)
                             {
                                 pretty_print(os, child, depth + 2);
                             });
        os << padding << ")\n";
    }
}
}

parse_tree::parse_tree(std::unique_ptr<node> root)
    : root_{std::move(root)}
{
    // nothing
}

std::ostream& operator<<(std::ostream& os, const parse_tree& tree)
{
    pretty_print(os, tree.root_.get(), 0);
    return os;
}
}
}
