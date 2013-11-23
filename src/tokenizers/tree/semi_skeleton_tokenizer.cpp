#include "tokenizers/tree/semi_skeleton_tokenizer.h"

namespace meta {
namespace tokenizers {

void semi_skeleton_tokenizer::tree_tokenize(corpus::document & doc,
        const parse_tree & tree)
{
    std::string representation = tree.get_category() + tree.skeleton();
    doc.increment(representation, 1);
    for(auto & child: tree.children())
        tree_tokenize(doc, child);

}

}
}
