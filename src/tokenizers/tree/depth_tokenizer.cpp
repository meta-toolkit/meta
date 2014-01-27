#include "tokenizers/tree/depth_tokenizer.h"
#include "util/common.h"

namespace meta {
namespace tokenizers {

void depth_tokenizer::tree_tokenize( corpus::document & doc,
        const parse_tree & tree)
{
    size_t h = parse_tree::height(tree);
    std::string representation = std::to_string(h);
    doc.increment(representation, 1);
}

}
}
