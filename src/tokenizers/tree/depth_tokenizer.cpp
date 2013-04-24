#include "tokenizers/tree/depth_tokenizer.h"

namespace meta {
namespace tokenizers {

void depth_tokenizer::tree_tokenize( index::document & document,
                                     const parse_tree & tree,
                                     mapping_fn mapping,
                                     const doc_freq_ptr & docFreq ) {
    size_t h = parse_tree::height(tree);
    std::string representation = common::to_string(h);
    document.increment(mapping(representation), 1, docFreq);
}

}
}
