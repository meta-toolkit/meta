#include "analyzers/tree/branch_analyzer.h"

namespace meta
{
namespace analyzers
{

const std::string branch_analyzer::id = "branch";

void branch_analyzer::tree_tokenize(corpus::document& doc,
                                    const parse_tree& tree)
{
    std::string representation = std::to_string(tree.num_children());
    doc.increment(representation, 1);
    for (auto& child : tree.children())
        tree_tokenize(doc, child);
}
}
}
