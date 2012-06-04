/**
 * @file pos_tokenizer.h
 */

#ifndef _POS_TREE_TOKENIZER_H_
#define _POS_TREE_TOKENIZER_H_

#include <unordered_map>
#include "document.h"
#include "parse_tree.h"
#include "tokenizer.h"

using std::unordered_map;

/**
 * 
 */
class POSTreeTokenizer : public Tokenizer
{
    public:
        /**
         *
         */
        void tokenize(const string & filename, Document & document) const;
};

#endif
