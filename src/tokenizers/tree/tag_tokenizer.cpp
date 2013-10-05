#include "tokenizers/tree/tag_tokenizer.h"

namespace meta {
namespace tokenizers {

void tag_tokenizer::tree_tokenize( corpus::document & document,
                                   const parse_tree & tree,
                                   mapping_fn mapping)
{
    std::string representation = tree.get_category();
    document.increment(mapping(representation), 1);
    for(auto & child: tree.children())
        tree_tokenize(document, child, mapping);
}

}
}
