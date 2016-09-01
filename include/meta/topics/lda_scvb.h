/**
 * @file lda_scvb.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_LDA_SCVB_H_
#define META_TOPICS_LDA_SCVB_H_

#include "meta/config.h"
#include "meta/topics/lda_model.h"

namespace meta
{
namespace topics
{

/**
 * lda_scvb: An implementation of LDA that uses stochastic collapsed
 * variational Bayes for inference. Specifically, it uses the SCVB0
 * algorithm detailed in Foulds et. al.
 *
 * @see http://dl.acm.org/citation.cfm?id=2487575.2487697
 */
class lda_scvb : public lda_model
{
  public:
    /**
     * Constructs the lda model over the given documents, with the
     * given number of topics, and hyperparameters \f$\alpha\f$ and
     * \f$\beta\f$ for the priors on \f$\phi\f$ (topic distributions)
     * and \f$\theta\f$ (topic proportions), respectively. Adheres to a
     * step-size schedule of \f$\frac{s}{(\tau + t)^\kappa}\f$.
     *
     * @param idx The index containing the documents to model
     * @param num_topics The number of topics to infer
     * @param alpha The hyperparameter for the Dirichlet prior over
     *  \f$\phi\f$
     * @param beta The hyperparameter for the Dirichlet prior over
     *  \f$\theta\f$
     * @param minibatch_size The number of documents to consider in a
     * minibatch
     */
    lda_scvb(std::shared_ptr<index::forward_index> idx, uint64_t num_topics,
             double alpha, double beta, uint64_t minibatch_size = 100);

    /**
     * Destructor: virtual for potential subclassing.
     */
    virtual ~lda_scvb() = default;

    /**
     * Runs the variational inference algorithm for a maximum number of
     * iterations.
     *
     * TODO: Is there a convenient convergence criterion for SCVB0?
     *
     * @param num_iters The maximum number of iterations (in terms of
     * minibatches) to run the inference algorithm for
     * @param convergence Unused
     */
    virtual void run(uint64_t num_iters, double convergence = 0) override;

    virtual double
    compute_term_topic_probability(term_id term, topic_id topic) const override;

    virtual double compute_doc_topic_probability(doc_id doc,
                                                 topic_id topic) const override;

  private:
    /**
     * Initialize the model with random parameters.
     *
     * @param gen The random number generator to use.
     */
    void initialize(std::mt19937& gen);

    /**
     * Performs one iteration (e.g., one minibatch) of the inference algorithm.
     * @param iter The iteration number
     * @param docs Contains the minibatch in indexes [0, minibatch_size_]
     */
    void perform_iteration(uint64_t iter, const std::vector<doc_id>& docs);

    /**
     * Contains the expected counts for each word being assigned a given
     * topic.  Indexed as `topic_term_count_[k][w]` where `k` is a
     * `topic_id` and `w` is a `term_id`.
     */
    std::vector<std::vector<double>> topic_term_count_;

    /**
     * Contains the expected counts for each topic being assigned in a
     * given document. Indexed as `doc_topic_count_[d][k]` where `d` is a
     * `doc_id` and `k` is a `topic_id`.
     */
    std::vector<std::vector<double>> doc_topic_count_;

    /**
     * Contains the expected number of times the given topic has been
     * assigned to a word. Can be inferred from the above maps, but is
     * included here for performance reasons.
     */
    std::vector<double> topic_count_;

    /// The hyperparameter on \f$\theta\f$, the topic proportions
    const double alpha_;
    /// The hyperparameter on \f$\phi\f$, the topic distributions
    const double beta_;
    /// The size of the minibatches
    const uint64_t minibatch_size_;
};
}
}

#endif
