/**
 * @file ngram_distribution
 */

#ifndef _NGRAM_DISTRIBUTION_
#define _NGRAM_DISTRIBUTION_

#include <iostream>
#include <unordered_map>
#include <utility>
#include "index/document.h"
#include "tokenizers/ngram_tokenizer.h"

template <size_t N>
class NgramDistribution
{
    public:

        typedef std::unordered_map<std::string, unsigned int> FreqMap;

        /**
         * Constructor.
         * @param docPath The training document.
         */
        NgramDistribution(const std::string & docPath);

    private:

        /**
         * Tokenizes a training document, counting frequencies of ngrams.
         */
        void calc_freqs();

        /**
         * Calculate D = n1 / (n1 + 2 * n2).
         * n1 = number of bigrams that appear exactly once.
         * n2 = number of bigrams that appear exactly twice.
         */
        void calc_discount_factor();

        /** Distribution for this ngram */
        FreqMap _dist;

        /** Discounting factor for this distribution */
        double _discount;
};

#include "ngram_distribution.cpp"
#endif
