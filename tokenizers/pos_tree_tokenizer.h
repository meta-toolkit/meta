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
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        void tokenize(Document & document, unordered_map<TermID, unsigned int>* docFreq);
};

#endif
