/**
 * @file tokenizer.h
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "document.h"
#include "parse_tree.h"

/**
 * An abstract class that takes a ParseTree and produces tokens.
 */
class ParseTreeTokenizer
{
    public:
        /**
         * Tokenizes a ParseTree, accumulating tokens in a document.
         * @param tree - the tree to tokenize
         * @param document - where to store aggregated tokens
         */
        virtual void tokenize(const ParseTree & tree, Document & document) const = 0;
};

#endif
