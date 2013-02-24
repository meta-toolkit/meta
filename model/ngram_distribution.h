/**
 * @file ngram_distribution
 * Declaration of a smoothed ngram language model class.
 */

#ifndef _NGRAM_DISTRIBUTION_
#define _NGRAM_DISTRIBUTION_

#include <iostream>
#include <unordered_map>
#include <utility>
#include "index/document.h"
#include "tokenizers/ngram_tokenizer.h"

/**
 * Represents a smoothed distribution of ngrams of either words, POS tags,
 * function words, or characters. Smoothing is done with absolute discounting
 * with the (n-1)-gram model recursively to the unigram level.
 */
template <size_t N>
class NgramDistribution
{
    public:

        typedef std::unordered_map<std::string, unsigned int> FreqMap;
        typedef std::unordered_map<std::string, double> ProbMap;

        /**
         * Constructor.
         * @param docPath The training document.
         */
        NgramDistribution(const std::string & docPath);

    private:

        /**
         * Tokenizes a training document, counting frequencies of ngrams.
         * @param docPath The training document
         */
        void calc_freqs(const std::string & docPath);

        /**
         * Tokenizes a training document, counting frequencies of ngrams.
         */
        void calc_dist();

        /**
         * Calculate D = n1 / (n1 + 2 * n2).
         * n1 = number of ngrams that appear exactly once.
         * n2 = number of ngrams that appear exactly twice.
         */
        void calc_discount_factor();

        /** Distribution for this ngram */
        FreqMap _freqs;

        /** Distribution for this ngram */
        ProbMap _dist;

        /** Distribution for this ngram */
        NgramDistribution<N - 1> _lower;

        /** Discounting factor for this distribution */
        double _discount;
};

/**
 * Specialized "base case" class for a unigram model's prior distribution.
 */
template <>
class NgramDistribution<0>
{
    public:
    
        typedef std::unordered_map<std::string, double> ProbMap;

        NgramDistribution(const std::string & docPath):
            _dist(ProbMap()){ /* nothing */ }
    
    private:
        
        ProbMap _dist;
};

#include "ngram_distribution.cpp"
#endif
