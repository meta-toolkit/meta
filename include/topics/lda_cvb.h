/**
 * @file lda_cvb.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_LDA_CVB_H_
#define META_TOPICS_LDA_CVB_H_

#include "topics/lda_model.h"

namespace meta
{
namespace topics
{

/**
 * lda_cvb: An implementation of LDA that uses collapsed variational bayes
 * for inference. Specifically, it uses the CVB0 algorithm detailed in
 * Asuncion et. al.
 *
 * @see http://www.ics.uci.edu/~asuncion/pubs/UAI_09.pdf
 */
class lda_cvb : public lda_model
{
  public:
    /**
     * Constructs the lda model over the given documents, with the
     * given number of topics, and hyperparameters \f$\alpha\f$ and
     * \f$\beta\f$ for the priors on \f$\phi\f$ (topic distributions)
     * and \f$\theta\f$ (topic proportions), respectively.
     *
     * @param idx The index containing the documents to model
     * @param num_topics The number of topics to infer
     * @param alpha The hyperparameter for the Dirichlet prior over
     *  \f$\phi\f$
     * @param beta The hyperparameter for the Dirichlet prior over
     *  \f$\theta\f$
     */
    lda_cvb(std::shared_ptr<index::forward_index> idx, uint64_t num_topics,
            double alpha, double beta);

    /**
     * Destructor: virtual for potential subclassing.
     */
    virtual ~lda_cvb() = default;

    /**
     * Runs the variational inference algorithm for a maximum number of
     * iterations, or until the given convergence criterion is met. The
     * convergence criterion is determined as the maximum difference in
     * any of the variational parameters \f$\gamma_{dij}\f$ in a given
     * iteration.
     *
     * @param num_iters The maximum number of iterations to run the
     *  sampler for
     * @param convergence The lowest maximum difference in any
     *  \f$\gamma_{dij}\f$ to be allowed before considering the
     *  inference to have converged
     */
    void run(uint64_t num_iters, double convergence = 1e-3);

  protected:
    /**
     * Initializes the parameters randomly.
     */
    void initialize();

    /**
     * Performs one iteration of the inference algorithm.
     *
     * @param iter The current iteration number
     * @return the maximum change in any of the \f$\gamma_{dij}\f$s
     */
    double perform_iteration(uint64_t iter);

    virtual double compute_term_topic_probability(term_id term,
                                                  topic_id topic) const
        override;

    virtual double compute_doc_topic_probability(doc_id doc,
                                                 topic_id topic) const override;

    /**
     * Variational parameters \f$\gamma_{ijk}\f$, which represent the soft
     * topic assignments for each word occurrence \f$i\f$ in document
     * \f$j\f$ to topic \f$k\f$. Actually indexed as `gamma_[d][i][k]`,
     * where `d` is a `doc_id`, `i` is the intra-document term id, and `k`
     * is a `topic_id`.
     */
    std::vector<std::vector<std::vector<double>>> gamma_;

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

    /**
     * Hyperparameter on \f$\theta\f$, the topic proportions.
     */
    const double alpha_;

    /**
     * Hyperparameter on \f$\phi\f$, the topic distributions.
     */
    const double beta_;
};
}
}

#endif
