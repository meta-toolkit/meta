/**
 * @file pos_tree_tokenizer.h
 */

#ifndef _POS_TREE_TOKENIZER_H_
#define _POS_TREE_TOKENIZER_H_

#include <unordered_map>
#include "index/document.h"
#include "parse_tree.h"
#include "tokenizer.h"

using std::unordered_map;

/**
 * Tokenizes a parse tree by the leaf POS values.
 */
class POSTreeTokenizer : public Tokenizer
{
    public:
        /**
         *
         */
        void tokenize(const string & filename, Document & document, unordered_map<string, size_t>* docFreq) const;
};

#endif
