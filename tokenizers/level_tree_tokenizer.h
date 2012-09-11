/**
 * @file level_tree_tokenizer.h
 */

#ifndef _LEVEL_TREE_TOKENIZER_H_
#define _LEVEL_TREE_TOKENIZER_H_

#include <memory>
#include <unordered_map>
#include "tokenizer.h"

class Document;

/**
 * Tokenizes parse trees based on their levels.
 */
class LevelTreeTokenizer : public Tokenizer
{
    public:
        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        void tokenize(Document & document,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);
};

#endif
