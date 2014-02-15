#include "analyzers/tree/depth_analyzer.h"

namespace meta {
namespace analyzers {

void depth_analyzer::tree_tokenize( corpus::document & doc,
        const parse_tree & tree)
{
    size_t h = parse_tree::height(tree);
    std::string representation = std::to_string(h);
    doc.increment(representation, 1);
}

}
}
