#include "tokenizers/tree/subtree_tokenizer.h"

namespace meta {
namespace tokenizers {

void subtree_tokenizer::tree_tokenize( corpus::document & document,
                                       const parse_tree & tree,
                                       mapping_fn mapping)
{
    std::string representation = tree.get_children_string() + "|" + tree.get_category();
    document.increment(mapping(representation), 1);
    for(auto & child: tree.children())
        tree_tokenize(document, child, mapping);
}

}
}
