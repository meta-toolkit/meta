/**
 * @file tree_tokenizer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _TREE_TOKENIZER_H_
#define _TREE_TOKENIZER_H_

#include "corpus/document.h"
#include "tokenizers/tokenizer.h"
#include "tokenizers/tree/parse_tree.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees with various methods.
 */
template <class DerivedTokenizer>
class tree_tokenizer: public tokenizer
{
    public:
        /**
         * Tokenizes a file into a document.
         * @param do The document to store the tokenized information in
         */
        void tokenize(corpus::document & doc) override
        {
            std::vector<parse_tree> trees =
                parse_tree::get_trees(doc.path() + ".tree");
            for(auto & tree: trees)
                derived().tree_tokenize(doc, tree);
        }

    private:
        /**
         * Convenience method to obtain this tokenizer as its derived class.
         */
        DerivedTokenizer & derived()
        {
            return static_cast<DerivedTokenizer &>( *this );
        }
};

}
}

#endif
