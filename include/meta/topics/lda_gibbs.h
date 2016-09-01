/**
 * @file topics/lda_gibbs.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LDA_GIBBS_H_
#define META_LDA_GIBBS_H_

#include <random>

#include "meta/config.h"
#include "meta/stats/multinomial.h"
#include "meta/topics/lda_model.h"
#include "meta/util/dense_matrix.h"

namespace meta
{
namespace topics
{

/**
 * A LDA topic model implemented using a collapsed gibbs sampler.
 *
 * @see http://www.pnas.org/content/101/suppl.1/5228.full.pdf
 */
class lda_gibbs : public lda_model
{
  public:
    /**
     * Constructs the lda model over the given documents, with the
     * given number of topics, and hyperparameters \f$\alpha\f$ and
     * \f$\beta\f$ for the priors on \f$\phi\f$ (topic distributions)
     * and \f$\theta\f$ (topic proportions), respectively.
     *
     * @param idx The index that contains the documents to model
     * @param num_topics The number of topics to infer
     * @param alpha The hyperparameter for the Dirichlet prior over
     * \f$\phi\f$
     * @param beta The hyperparameter for the Dirichlet prior over
     * \f$\theta\f$
     */
    lda_gibbs(std::shared_ptr<index::forward_index> idx, uint64_t num_topics,
              double alpha, double beta);

    /**
     * Destructor: virtual for potential subclassing.
     */
    virtual ~lda_gibbs() = default;

    /**
     * Runs the sampler for a maximum number of iterations, or until
     * the given convergence criterion is met. The convergence
     * criterion is determined as the relative difference in log
     * corpus likelihood between two iterations.
     *
     * @param num_iters The maximum number of iterations to run the
     * sampler for
     * @param convergence The lowest relative difference in \f$\log
     * P(\mathbf{w} \mid \mathbf{z})\f$ to be allowed before considering
     * the sampler to have converged
     */
    virtual void run(uint64_t num_iters, double convergence = 1e-6) override;

    /**
     * @return the probability that the given term appears in the given
     * topic
     *
     * @param term The term we are concerned with.
     * @param topic The topic we are concerned with.
     */
    virtual double
    compute_term_topic_probability(term_id term, topic_id topic) const override;

    /**
     * @return the probability that the given topic is picked for the given
     * document
     *
     * @param doc The document we are concerned with.
     * @param topic The topic we are concerned with.
     */
    virtual double compute_doc_topic_probability(doc_id doc,
                                                 topic_id topic) const override;

  protected:
    /**
     * Samples a topic from the full conditional distribution
     * \f$P(z_i = j | w, \boldsymbol{z})\f$. Used in both initialization
     * and each normal iteration of the sampler, after removing the
     * current value of \f$z_i\f$ from the vector of assignments
     * \f$\boldsymbol{z}\f$.
     *
     * @param term The term we are sampling a topic assignment for
     * @param doc The document the term resides in
     * @return the topic sampled the given (term, doc) pair
     */
    topic_id sample_topic(term_id term, doc_id doc);

    /**
     * Computes a weight proportional to \f$P(z_i = j | w, \boldsymbol{z})\f$.
     *
     * @param term The current word we are sampling for
     * @param doc The document in which the term resides
     * @param topic The topic \f$j\f$ we want to compute the
     * probability for
     * @return a weight proportional to the probability that the given term
     * in the given document belongs to the given topic
     */
    virtual double compute_sampling_weight(term_id term, doc_id doc,
                                           topic_id topic) const;

    /**
     * Initializes the first set of topic assignments for inference.
     * Employs an online application of the sampler where counts are
     * only considered for the words observed so far through the loop.
     */
    virtual void initialize();

    /**
     * Performs a sampling iteration.
     *
     * @param iter The iteration number
     * @param init Whether or not to employ the online method (defaults to
     * `false`)
     */
    virtual void perform_iteration(uint64_t iter, bool init = false);

    /**
     * Decreases all counts associated with the given topic, term, and
     * document by one.
     *
     * @param topic The topic in question
     * @param term The term in question
     * @param doc The document in question
     */
    virtual void decrease_counts(topic_id topic, term_id term, doc_id doc);

    /**
     * Increases all counts associated with the given topic, term, and
     * document by one.
     *
     * @param topic The topic in question
     * @param term The term in question
     * @param doc The document in question
     */
    virtual void increase_counts(topic_id topic, term_id term, doc_id doc);

    /**
     * @return \f$\log P(\mathbf{w} \mid \mathbf{z})\f$
     */
    double corpus_log_likelihood() const;

    /**
     * lda_gibbs cannot be copy assigned.
     */
    lda_gibbs& operator=(const lda_gibbs&) = delete;

    /**
     * lda_gibbs cannot be copy constructed.
     */
    lda_gibbs(const lda_gibbs& other) = delete;

    /**
     * The topic assignment for every word in every document. Note that
     * the same word occurring multiple times in one document could
     * potentially have many different topics assigned to it, so we are
     * not using term_ids here, but our own contrived intra document term id.
     *
     * Indexed as [doc_id][position].
     */
    std::vector<std::vector<topic_id>> doc_word_topic_;

    /**
     * The word distributions for each topic, \f$\phi_t\f$.
     */
    std::vector<stats::multinomial<term_id>> phi_;

    /**
     * The topic distributions for each document, \f$\theta_d\f$.
     */
    std::vector<stats::multinomial<topic_id>> theta_;

    /**
     * The random number generator for the sampler.
     */
    std::mt19937_64 rng_;
};
}
}

#endif
