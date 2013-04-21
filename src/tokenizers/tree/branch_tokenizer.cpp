#include "tokenizers/tree/branch_tokenizer.h"

namespace meta {
namespace tokenizers {

void branch_tokenizer::tree_tokenize( index::Document & document,
                                      const ParseTree & tree,
                                      mapping_fn mapping,
                                      const doc_freq_ptr & docFreq ) {
    std::string representation = common::to_string(tree.numChildren());
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        tree_tokenize(document, child, mapping, docFreq);
}

}
}
