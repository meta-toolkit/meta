/**
 * @file level_tokenizer.h
 */

#ifndef _LEVEL_TOKENIZER_H_
#define _LEVEL_TOKENIZER_H_

#include "document.h"
#include "parse_tree.h"
#include "tokenizer.h"

/**
 * 
 */
class LevelTokenizer : public ParseTreeTokenizer
{
    public:
        /**
         * Tokenizes a ParseTree, accumulating tokens in a document.
         * @param tree - the tree to tokenize
         * @param document - where to store aggregated tokens
         */
        void tokenize(const ParseTree & tree, Document & document) const;
};

#endif
