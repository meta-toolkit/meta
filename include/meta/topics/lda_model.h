/**
 * @file topics/lda_model.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_LDA_MODEL_H_
#define META_TOPICS_LDA_MODEL_H_

#include "meta/config.h"
#include "meta/index/forward_index.h"

MAKE_NUMERIC_IDENTIFIER(topic_id, uint64_t)

namespace meta
{
namespace topics
{

/**
 * An LDA topic model base class.
 *
 * Required config parameters (for use with the ./lda executable):
 * ~~~toml
 * inference = "inference-method" # gibbs, pargibbs, cvb, scvb
 * max-iters = 1000
 * alpha = 1.0
 * beta = 1.0
 * topics = 4
 * model-prefix = "prefix"
 * ~~~
 *
 * Optional config parameters: none.
 */
class lda_model
{
  public:
    /**
     * Constructs an lda_model over the given set of documents and with a
     * fixed number of topics.
     *
     * @param idx The index containing the documents to use for the model
     * @param num_topics The number of topics to find
     */
    lda_model(std::shared_ptr<index::forward_index> idx, uint64_t num_topics);

    /**
     * Destructor. Made virtual to allow for deletion through pointer to
     * base.
     */
    virtual ~lda_model() = default;

    /**
     * Runs the model for a given number of iterations, or until a
     * convergence criteria is met.
     *
     * @param num_iters The maximum allowed number of iterations
     * @param convergence The convergence criteria (this has different
     * meanings for different subclass models)
     */
    virtual void run(uint64_t num_iters, double convergence) = 0;

    /**
     * Saves the topic proportions \f$\theta_d\f$ for each document to
     * the given file. Saves the distributions in a simple "human
     * readable" plain-text format.
     *
     * @param filename The file to save \f$\theta\f$ to
     */
    void save_doc_topic_distributions(const std::string& filename) const;

    /**
     * Saves the term distributions \f$\phi_j\f$ for each topic to the
     * given file. Saves the distributions in a simple "human readable"
     * plain-text format.
     *
     * @param filename The file to save \f$\phi\f$ to
     */
    void save_topic_term_distributions(const std::string& filename) const;

    /**
     * Saves the current model to a set of files beginning with prefix:
     * prefix.phi, prefix.theta, and prefix.terms.
     *
     * @param prefix The prefix for all generated files over this model
     */
    void save(const std::string& prefix) const;

    /**
     * @return the probability that the given term appears in the given
     * topic
     *
     * @param term The term we are concerned with
     * @param topic The topic we are concerned with
     */
    virtual double compute_term_topic_probability(term_id term,
                                                  topic_id topic) const = 0;

    /**
     * @return the probability that the given topic is picked for the given
     * document
     *
     * @param doc The document we are concerned with
     * @param topic The topic we are concerned with
     */
    virtual double compute_doc_topic_probability(doc_id doc,
                                                 topic_id topic) const = 0;

    /**
     * @return the number of topics in this model
     */
    uint64_t num_topics() const;

  protected:
    /**
     * lda_models cannot be copy assigned.
     */
    lda_model& operator=(const lda_model&) = delete;

    /**
     * lda_models cannot be copy constructed.
     */
    lda_model(const lda_model&) = delete;

    /**
     * The index containing the documents for the model.
     */
    std::shared_ptr<index::forward_index> idx_;

    /**
     * The number of topics.
     */
    size_t num_topics_;

    /**
     * The number of total unique words.
     */
    size_t num_words_;
};
}
}

#endif
