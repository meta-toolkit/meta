/**
 * @file pos_tokenizer.h
 */

#ifndef _NGRAM_TOKENIZER_H_
#define _NGRAM_TOKENIZER_H_

#include <vector>
#include <unordered_map>
#include "document.h"
#include "parse_tree.h"
#include "tokenizer.h"

using std::vector;
using std::unordered_map;

/**
 * 
 */
class NgramTokenizer : public Tokenizer
{
    public:
        /**
         * Constructor.
         * @param n - the value of n to use for the ngrams.
         */
        NgramTokenizer(size_t n): _nValue(n){/* nothing */}

        /**
         *
         */
        void tokenize(const string & filename, Document & document) const;

        /**
         * @return the value of n used for the ngrams
         */
        size_t getNValue() const;

    private:

        size_t _nValue;
};

#endif
