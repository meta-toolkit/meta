#include "tokenizers/tree/tag_tokenizer.h"

namespace meta {
namespace tokenizers {

void tag_tokenizer::tree_tokenize( corpus::document & doc,
        const parse_tree & tree)
{
    std::string representation = tree.get_category();
    doc.increment(representation, 1);
    for(auto & child: tree.children())
        tree_tokenize(doc, child);
}

}
}
