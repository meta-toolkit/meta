/**
 * @file tree_tokenizer.h
 */

#ifndef _LEVEL_TREE_TOKENIZER_H_
#define _LEVEL_TREE_TOKENIZER_H_

#include <string>
#include <memory>
#include <unordered_map>
#include "tokenizer.h"

class ParseTree;
class Document;

/**
 * Tokenizes parse trees.
 */
class TreeTokenizer : public Tokenizer
{
    public:
        /**
         *
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

        /** Hashes specific tree tokenizer types to tokenizer functions */
        typedef std::function<void(Document &, const ParseTree &, std::shared_ptr<std::unordered_map<TermID, unsigned int>>)> TokenizerFunction;
        std::unordered_map< TreeTokenizerType, TokenizerFunction, std::hash<int> > _tokenizerTypes;

        /**
         * @param document
         * @param tree
         * @param docFreq 
         */
        void depthTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

        /**
         * @param document
         * @param tree
         * @param docFreq 
         */
        void subtreeTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

        /**
         * @param document
         * @param tree
         * @param docFreq 
         */
        void tagTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

        /**
         * @param document
         * @param tree
         * @param docFreq 
         */
        void branchTokenize(Document & document, const ParseTree & tree,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);
};

#endif
