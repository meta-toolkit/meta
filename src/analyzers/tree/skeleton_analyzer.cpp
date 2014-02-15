#include "analyzers/tree/skeleton_analyzer.h"

namespace meta {
namespace analyzers {

void skeleton_analyzer::tree_tokenize( corpus::document & doc,
        const parse_tree & tree)
{
    std::string representation = tree.skeleton();
    doc.increment(representation, 1);
    for(auto & child: tree.children())
        tree_tokenize(doc, child);

}

}
}
