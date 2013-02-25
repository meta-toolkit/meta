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

        typedef std::unordered_map<std::string, std::unordered_map<std::string, size_t>> FreqMap;
        typedef std::unordered_map<std::string, std::unordered_map<std::string, double>> ProbMap;

        /**
         * Constructor.
         * @param docPath The training document.
         */
        NgramDistribution(const std::string & docPath);

        /**
         * @param prev
         * @param word
         * @return the probability of seeing "prev+word".
         */
        double prob(const std::string & prev, const std::string & word) const;

        /**
         * @param str
         * @return the probability of seeing "str".
         */
        double prob(const std::string & str) const;

    private:

        /**
         * Tokenizes a training document, counting frequencies of ngrams.
         * @param docPath The training document
         */
        void calc_freqs(const std::string & docPath);

        /**
         * Calculates a smoothed probability distribution over ngrams.
         * \f$P_{AD}(word|prev) = \frac{ max(c(prevword) - D, 0) }{ c(prev) } +
         *  \frac{D}{c(prev)} \cdot |S_w| \cdot P_{AD}(word)\f$
         */
        void calc_dist();

        /**
         * Calculate D = n1 / (n1 + 2 * n2).
         * n1 = number of ngrams that appear exactly once.
         * n2 = number of ngrams that appear exactly twice.
         */
        void calc_discount_factor();

        /**
         * @param words A string "w1 w2 w3 w4"
         * @return "w4" from "w1 w2 w3 w4"
         */
        std::string get_last(const std::string & words) const;

        /**
         * @param words A string "w1 w2 w3 w4"
         * @return "w1 w2 w3" from "w1 w2 w3 w4"
         */
        std::string get_rest(const std::string & words) const;

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
    
        typedef std::unordered_map<std::string, std::unordered_map<std::string, double>> ProbMap;

        NgramDistribution(const std::string & docPath):
            _dist(ProbMap()){ /* nothing */ }

        double prob(const std::string & str)
        {
            return 0.0;
        }
    
    private:
        
        ProbMap _dist;
};

#include "ngram_distribution.cpp"
#endif
