#include "tokenizers/tree/skeleton_tokenizer.h"

namespace meta {
namespace tokenizers {

void skeleton_tokenizer::tree_tokenize( index::document & document,
                                        const ParseTree & tree,
                                        mapping_fn mapping,
                                        const doc_freq_ptr & docFreq ) {
    std::string representation = tree.getSkeleton();
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        tree_tokenize(document, child, mapping, docFreq);

}

}
}
