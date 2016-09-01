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

#include "meta/config.h"
#include "meta/stats/multinomial.h"
#include "meta/topics/lda_model.h"

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
    void run(uint64_t num_iters, double convergence = 1e-3) override;

    virtual double
    compute_term_topic_probability(term_id term, topic_id topic) const override;

    virtual double compute_doc_topic_probability(doc_id doc,
                                                 topic_id topic) const override;

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

    /**
     * Variational distributions \f$\gamma_{ij}\f$, which represent the soft
     * topic assignments for each word occurrence \f$i\f$ in document
     * \f$j\f$.
     *
     * Indexed as gamma_[d][i]
     */
    std::vector<std::vector<stats::multinomial<topic_id>>> gamma_;

    /**
     * The word distributions for each topic, \f$\phi_t\f$.
     */
    std::vector<stats::multinomial<term_id>> phi_;

    /**
     * The topic distributions for each document, \f$\theta_d\f$.
     */
    std::vector<stats::multinomial<topic_id>> theta_;
};
}
}
#endif
