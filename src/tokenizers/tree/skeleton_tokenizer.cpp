#include "tokenizers/tree/skeleton_tokenizer.h"

namespace meta {
namespace tokenizers {

void skeleton_tokenizer::tree_tokenize( index::document & document,
                                        const parse_tree & tree,
                                        mapping_fn mapping)
{
    std::string representation = tree.skeleton();
    document.increment(mapping(representation), 1);
    for(auto & child: tree.children())
        tree_tokenize(document, child, mapping);

}

}
}
