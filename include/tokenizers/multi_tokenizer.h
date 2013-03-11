/**
 * @file multi_tokenizer.h
 */

#ifndef _MULTI_TOKENIZER_
#define _MULTI_TOKENIZER_

#include <vector>
#include "tokenizers/tokenizer.h"

/**
 * The MultiTokenizer class contains more than one tokenizer. This is useful for
 * trying combined feature methods.
 *
 * For example, you could tokenize based on ngrams of words and parse tree
 * rewrite rules. The MultiTokenizer keeps track of all the features in one set
 * for however many internal Tokenizers it contains.
 */
class MultiTokenizer: public Tokenizer
{
    public:

        /**
         * Constructs a MultiTokenizer from a vector of other Tokenizers.
         */
        MultiTokenizer(const std::vector<Tokenizer*> & tokenizers);

        /**
         * Destructor; we need to free the collection of other Tokenizers.
         */
        ~MultiTokenizer();

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        void tokenize(Document & document,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq = nullptr);

    private:

        /** Holds all the Tokenizers in this MultiTokenizer */
        std::vector<Tokenizer*> _tokenizers;

        /**
         * Disallow copying.
         */
        MultiTokenizer(const MultiTokenizer & other) = delete;

        /**
         * Disallow assignment.
         */
        const MultiTokenizer & operator=(const MultiTokenizer & other) = delete;
};

#endif
