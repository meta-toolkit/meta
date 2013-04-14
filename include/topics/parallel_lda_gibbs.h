/**
 * @file topics/parallel_lda_gibbs.h
 */

#ifndef _DST_PARALLEL_LDA_GIBBS_H_
#define _DST_PARALLEL_LDA_GIBBS_H_

#include <thread>

#include "parallel/parallel_for.h"
#include "topics/lda_gibbs.h"
#include "util/range.h"

namespace meta {
namespace topics {

/**
 * An LDA topic model implemented using the Approximate Distributed LDA
 * algorithm. Based on the algorithm detailed by David Newman et. al.
 * 
 * @see http://www.jmlr.org/papers/volume10/newman09a/newman09a.pdf
 */
class parallel_lda_gibbs : public lda_gibbs {
    public:
        /**
         * Constructs the lda model over the given documents, with the
         * given number of topics, and hyperparameters \f$\alpha\f$ and
         * \f$\beta\f$ for the priors on \f$\phi\f$ (topic distributions)
         * and \f$\theta\f$ (topic proportions), respectively.
         *
         * Assumes that the given vector of documents will live for as long
         * as or longer than the parallel_lda_gibbs instance.
         *
         * @param docs The documents for the LDA model
         * @param num_topics The number of topics to infer
         * @param alpha The hyperparameter for the Dirichlet prior over
         *  \f$\phi\f$.
         * @param beta The hyperparameter for the Dirichlet prior over
         *  \f$\theta\f$.
         */
        parallel_lda_gibbs( std::vector<index::Document> & docs, size_t num_topics,
                            double alpha, double beta );

        /**
         * Destructor: virtual for potential subclassing.
         */
        virtual ~parallel_lda_gibbs();

    protected:

        virtual void initialize();

        /**
         * Performs a sampling iteration of the AD-LDA algorithm. This
         * consists of splitting up the sampling of (document, word) topic
         * assignments across threads, keeping for each thread a difference
         * in counts for the potentially shared topic counts. Once the
         * sampling has finished, the counts are reduced down (serially)
         * before the iteration is completed.
         */
        virtual void perform_iteration( bool init = false );

        virtual void decrease_counts( size_t topic, term_id term, size_t doc );

        virtual void increase_counts( size_t topic, term_id term, size_t doc );

        virtual double count_term( term_id term, size_t topic ) const;

        virtual double count_topic( size_t topic ) const;

        /**
         * The thread pool used for parallelization.
         */
        parallel::thread_pool pool_;

        /**
         * The mutex used for synchronization around progress updates.
         */
        std::mutex mutex_;

        using topic_id = size_t; // for clarity below

        /**
         * Stores the difference in topic_term counts on a per-thread basis
         * for use in the reduction step.
         */
        std::unordered_map<
            std::thread::id, 
            std::unordered_map<topic_id, std::unordered_map<term_id, ssize_t>>
        > topic_term_diffs_;

        /**
         * Stores the difference in topic counts on a per-thread basis for
         * use in the reduction step.
         */
        std::unordered_map<
            std::thread::id,
            std::unordered_map<topic_id, ssize_t>
        > topic_diffs_;

};

}
}

#endif
