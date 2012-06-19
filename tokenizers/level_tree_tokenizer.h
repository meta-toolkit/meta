/**
 * @file level_tree_tokenizer.h
 */

#ifndef _LEVEL_TREE_TOKENIZER_H_
#define _LEVEL_TREE_TOKENIZER_H_

#include "index/document.h"
#include "parse_tree.h"
#include "tokenizer.h"

/**
 * Tokenizes parse trees based on their levels.
 */
class LevelTreeTokenizer : public Tokenizer
{
    public:
        /**
         * Tokenizes a file into a Document.
         * @param filename - the file to read tokens from
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        void tokenize(const string & filename, Document & document, unordered_map<TermID, unsigned int>* docFreq);
};

#endif
