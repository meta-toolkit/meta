#include "analyzers/tree/semi_skeleton_analyzer.h"

namespace meta {
namespace analyzers {

void semi_skeleton_analyzer::tree_tokenize(corpus::document & doc,
        const parse_tree & tree)
{
    std::string representation = tree.get_category() + tree.skeleton();
    doc.increment(representation, 1);
    for(auto & child: tree.children())
        tree_tokenize(doc, child);

}

}
}
