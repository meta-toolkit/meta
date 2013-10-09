#include "tokenizers/tree/branch_tokenizer.h"

namespace meta {
namespace tokenizers {

void branch_tokenizer::tree_tokenize( corpus::document & document,
                                      const parse_tree & tree,
                                      mapping_fn mapping)
{
    std::string representation = common::to_string(tree.num_children());
    document.increment(mapping(representation), 1);
    for(auto & child: tree.children())
        tree_tokenize(document, child, mapping);
}

}
}
