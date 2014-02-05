/**
 * @file topics/parallel_lda_gibbs.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DST_PARALLEL_LDA_GIBBS_H_
#define _DST_PARALLEL_LDA_GIBBS_H_

#include <thread>

#include "parallel/thread_pool.h"
#include "topics/lda_gibbs.h"

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
    virtual void initialize();

    /**
     * Performs a sampling iteration of the AD-LDA algorithm. This
     * consists of splitting up the sampling of (document, word) topic
     * assignments across threads, keeping for each thread a difference
     * in counts for the potentially shared topic counts. Once the
     * sampling has finished, the counts are reduced down (serially)
     * before the iteration is completed.
     */
    virtual void perform_iteration(bool init = false);

    virtual void decrease_counts(topic_id topic, term_id term,
                                 doc_id doc) override;

    virtual void increase_counts(topic_id topic, term_id term,
                                 doc_id doc) override;

    virtual double count_term(term_id term, topic_id topic) const;

    virtual double count_topic(topic_id topic) const;

    /**
     * The thread pool used for parallelization.
     */
    parallel::thread_pool pool_;

    /**
     * Stores the difference in topic_term counts on a per-thread basis
     * for use in the reduction step.
     */
    std::unordered_map<
        std::thread::id,
        std::unordered_map<topic_id, std::unordered_map<term_id, ssize_t>>>
        topic_term_diffs_;

    /**
     * Stores the difference in topic counts on a per-thread basis for
     * use in the reduction step.
     */
    std::unordered_map<std::thread::id, std::unordered_map<topic_id, ssize_t>>
        topic_diffs_;
};
}
}

#endif
