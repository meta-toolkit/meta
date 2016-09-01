/**
 * @file topics/parallel_lda_gibbs.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_PARALLEL_LDA_GIBBS_H_
#define META_PARALLEL_LDA_GIBBS_H_

#include <thread>

#include "meta/config.h"
#include "meta/parallel/thread_pool.h"
#include "meta/topics/lda_gibbs.h"

namespace meta
{
namespace topics
{

/**
 * An LDA topic model implemented using the Approximate Distributed LDA
 * algorithm. Based on the algorithm detailed by David Newman et. al.
 *
 * @see http://www.jmlr.org/papers/volume10/newman09a/newman09a.pdf
 */
class parallel_lda_gibbs : public lda_gibbs
{
  public:
    /** use same constructor from base class */
    using lda_gibbs::lda_gibbs;

    /**
     * Destructor: virtual for potential subclassing.
     */
    virtual ~parallel_lda_gibbs() = default;

  protected:
    virtual void initialize() override;

    /**
     * Performs a sampling iteration of the AD-LDA algorithm. This
     * consists of splitting up the sampling of (document, word) topic
     * assignments across threads, keeping for each thread a difference
     * in counts for the potentially shared topic counts. Once the
     * sampling has finished, the counts are reduced down (serially)
     * before the iteration is completed.
     *
     * @param iter The current iteration number
     * @param init Whether or not this iteration should use the online
     * method for initializing the sampler
     */
    virtual void perform_iteration(uint64_t iter, bool init = false) override;

    virtual void decrease_counts(topic_id topic, term_id term,
                                 doc_id doc) override;

    virtual void increase_counts(topic_id topic, term_id term,
                                 doc_id doc) override;

    virtual double compute_sampling_weight(term_id term, doc_id doc,
                                           topic_id topic) const override;

    /**
     * The thread pool used for parallelization.
     */
    parallel::thread_pool pool_;

    /**
     * Stores the difference in topic_term counts on a per-thread basis
     * for use in the reduction step.
     *
     * Indexed as [thread_id][topic]
     */
    std::unordered_map<std::thread::id,
                       std::vector<stats::multinomial<term_id>>>
        phi_diffs_;
};
}
}

#endif
