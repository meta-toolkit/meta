/**
 * @file tree_tokenizer.h
 */

#ifndef _TREE_TOKENIZER_H_
#define _TREE_TOKENIZER_H_

#include <string>
#include <memory>
#include <unordered_map>
#include "tokenizer.h"

class ParseTree;
class Document;

/**
 * Tokenizes parse trees with various methods.
 */
class TreeTokenizer: public Tokenizer
{
    public:
        /**
         * Enumeration representing different ways to tokenize parse trees.
         */
        enum TreeTokenizerType { Subtree, Depth, Branch, Tag };

        /**
         * Constructor.
         * @param type - The specific type of tree tokenizer employed
         */
        TreeTokenizer(TreeTokenizerType type);

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        void tokenize(Document & document,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

    private:

        /** the file extension for ParseTrees stored on disk */
        const static char* _extension;

        /** the type of tree tokenizer this tokenizer is using */
        TreeTokenizerType _type;

        /** Tree tokenizer function type */
        typedef std::function<void(Document &, const ParseTree &, std::shared_ptr<std::unordered_map<TermID, unsigned int>>)> TokenizerFunction;

        /** Hashes specific tree tokenizer types to tokenizer functions */
        std::unordered_map< TreeTokenizerType, TokenizerFunction, std::hash<int> > _tokenizerTypes;

        /**
         * Extracts the depth feature from parse trees: what are the heights of
         * a document's trees?
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void depthTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

        /**
         * Counts occurrences of subtrees in this document's ParseTrees.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void subtreeTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

        /**
         * Counts occurrences of leaf and interior node labels.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void tagTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

        /**
         * Keeps track of the branching factor for this document's ParseTrees.
         * @param document - the document to parse
         * @param tree - the current ParseTree in the document
         * @param docFreq - used to aggregate counts for this TreeTokenizer
         *  instance
         */
        void branchTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);
};

#endif
