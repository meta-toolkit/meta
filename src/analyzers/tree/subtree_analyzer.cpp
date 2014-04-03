#include "analyzers/tree/subtree_analyzer.h"

namespace meta
{
namespace analyzers
{

const std::string subtree_analyzer::id = "subtree";

void subtree_analyzer::tree_tokenize(corpus::document& doc,
                                     const parse_tree& tree)
{
    std::string representation = tree.get_children_string() + "|"
                                 + tree.get_category();
    doc.increment(representation, 1);
    for (auto& child : tree.children())
        tree_tokenize(doc, child);
}
}
}
