/**
 * @file topics/lda_model.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DST_TOPICS_LDA_MODEL_H_
#define _DST_TOPICS_LDA_MODEL_H_

#include "index/forward_index.h"

MAKE_NUMERIC_IDENTIFIER(topic_id, uint64_t);

namespace meta
{
namespace topics
{

/**
 * An LDA topic model base class.
 */
class lda_model
{
  public:
    lda_model(index::forward_index& idx, uint64_t num_topics);

    virtual ~lda_model() = default;

    virtual void run(uint64_t num_iters, double convergence) = 0;

    /**
     * Saves the topic proportions \f$\theta_d\f$ for each document to
     * the given file. Saves the distributions in a simple "human
     * readable" plain-text format.
     *
     * @param filename The file to save \f$theta\f$ to.
     */
    void save_doc_topic_distributions(const std::string& filename) const;

    /**
     * Saves the term distributions \f$\phi_j\f$ for each topic to the
     * given file. Saves the distributions in a simple "human readable"
     * plain-text format.
     *
     * @param filename The file to save \f$\phi\f$ to.
     */
    void save_topic_term_distributions(const std::string& filename) const;

    /**
     * Saves the current model to a set of files beginning with prefix:
     * prefix.phi, prefix.theta, and prefix.terms.
     *
     * @param prefix The prefix for all generated files over this
     *  model.
     */
    void save(const std::string& prefix) const;

  protected:
    lda_model& operator=(const lda_model&) = delete;
    lda_model(const lda_model&) = delete;

    /**
     * Computes the probability that the given term appears in the
     * given topic.
     *
     * @param term The term we are concerned with.
     * @param topic The topic we are concerned with.
     */
    virtual double compute_term_topic_probability(term_id term,
                                                  topic_id topic) const = 0;

    /**
     * Computes the probability that the given topic is picked for the
     * given document.
     *
     * @param doc The document we are concerned with.
     * @param topic The topic we are concerned with.
     */
    virtual double compute_doc_topic_probability(doc_id doc,
                                                 topic_id topic) const = 0;

    index::forward_index& idx_;
    size_t num_topics_;
    size_t num_words_;
};
}
}

#endif
