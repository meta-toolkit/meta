/**
 * @file tree_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _TREE_TOKENIZER_H_
#define _TREE_TOKENIZER_H_

#include <string>
#include <memory>
#include <unordered_map>
#include "tokenizers/tokenizer.h"
#include "tokenizers/parse_tree.h"
#include "index/document.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes parse trees with various methods.
 */
class tree_tokenizer: public tokenizer
{
    public:
        /**
         * Enumeration representing different ways to tokenize parse trees.
         */
        enum TreeTokenizerType { Subtree, Depth, Branch, Tag, Skeleton, SemiSkeleton, Multi };

        /**
         * Constructor.
         * @param type - The specific type of tree tokenizer employed
         */
        tree_tokenizer(TreeTokenizerType type);

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        void tokenize(index::Document & document,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq = nullptr);

    private:

        /** the type of tree tokenizer this tokenizer is using */
        TreeTokenizerType _type;

        /** Tree tokenizer function type */
        typedef std::function<void(index::Document &, const ParseTree &, std::shared_ptr<std::unordered_map<term_id, unsigned int>>)> TokenizerFunction;

        /** Hashes specific tree tokenizer types to tokenizer functions */
        std::unordered_map< TreeTokenizerType, TokenizerFunction, std::hash<int> > _tokenizer_types;

        /**
         * Extracts the depth feature from parse trees: what are the heights of
         * a document's trees?
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void depthTokenize(index::Document & document, const ParseTree & tree,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq);

        /**
         * Counts occurrences of subtrees in this document's ParseTrees.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void subtreeTokenize(index::Document & document, const ParseTree & tree,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq);

        /**
         * Counts occurrences of leaf and interior node labels.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void tagTokenize(index::Document & document, const ParseTree & tree,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq);

        /**
         * Keeps track of the branching factor for this document's ParseTrees.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void branchTokenize(index::Document & document, const ParseTree & tree,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq);

        /**
         * Ignores node labels and only tokenizes the tree structure.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void skeletonTokenize(index::Document & document, const ParseTree & tree,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq);

        /**
         * Keeps track of one node's tag and the skeleton structure beneath it.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void semiSkeletonTokenize(index::Document & document, const ParseTree & tree,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq);

        /**
         * Runs multiple tokenizers at once.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void multiTokenize(index::Document & document, const ParseTree & tree,
                const std::shared_ptr<std::unordered_map<term_id, unsigned int>> & docFreq);
};

}
}

#endif
