/**
 * @file pos_tokenizer.h
 */

#ifndef _POS_TOKENIZER_H_
#define _POS_TOKENIZER_H_

#include "document.h"
#include "parse_tree.h"
#include "tokenizer.h"

/**
 * 
 */
class POSTokenizer : public ParseTreeTokenizer
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
