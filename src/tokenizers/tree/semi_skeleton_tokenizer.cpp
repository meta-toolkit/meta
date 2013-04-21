#include "tokenizers/tree/semi_skeleton_tokenizer.h"

namespace meta {
namespace tokenizers {

void semi_skeleton_tokenizer::tree_tokenize( index::Document & document,
                                             const ParseTree & tree,
                                             mapping_fn mapping,
                                             const doc_freq_ptr & docFreq ) {
    std::string representation = tree.getPOS() + tree.getSkeleton();
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        tree_tokenize(document, child, mapping, docFreq);

}

}
}
