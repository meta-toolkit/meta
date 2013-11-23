#include "tokenizers/tree/subtree_tokenizer.h"

namespace meta {
namespace tokenizers {

void subtree_tokenizer::tree_tokenize( corpus::document & doc,
        const parse_tree & tree)
{
    std::string representation = tree.get_children_string() + "|" + tree.get_category();
    doc.increment(representation, 1);
    for(auto & child: tree.children())
        tree_tokenize(doc, child);
}

}
}
