/**
 * @file level_tokenizer.h
 */

#ifndef _LEVEL_TREE_TOKENIZER_H_
#define _LEVEL_TREE_TOKENIZER_H_

#include "document.h"
#include "parse_tree.h"
#include "tokenizer.h"

/**
 * 
 */
class LevelTreeTokenizer : public Tokenizer
{
    public:
        /**
         *
         */
        void tokenize(const string & filename, Document & document) const;
};

#endif
