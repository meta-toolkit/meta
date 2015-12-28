/**
 * @file scorer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_CRF_SCORER_H_
#define META_SEQUENCE_CRF_SCORER_H_

#include "meta/sequence/crf/crf.h"

namespace meta
{
namespace sequence
{

/**
 * Internal class that holds scoring information for sequences under
 * the current model.
 */
class crf::scorer
{
  public:
    /**
     * Finds both transition and state scores, in logarithm and
     * exponential domains.
     *
     * @param model The model to score with
     * @param seq The sequence to score
     */
    void score(const crf& model, const sequence& seq);

    /**
     * Finds only the transition scores.
     *
     * @param model The model to score with
     */
    void transition_scores(const crf& model);

    /**
     * Finds only the state scores.
     *
     * @param model The model to score with
     * @param seq The sequence to score
     */
    void state_scores(const crf& model, const sequence& seq);

    /**
     * Computes the forward trellis.
     */
    void forward();

    /**
     * Computes the backward trellis.
     */
    void backward();

    /**
     * Computes the marginal probabilities of states and transitions
     * by using the forward-backward algorithm results.
     */
    void marginals();

    /**
     * @param time The time step
     * @param lbl The state
     * @return the log-domain score for being in this state at the
     * given time
     */
    double state(uint64_t time, label_id lbl) const;

    /**
     * @param time The time step
     * @param lbl The state
     * @return the score for being in this state at the given time
     */
    double state_exp(uint64_t time, label_id lbl) const;

    /**
     * @param from The origin state
     * @param to The destination state
     * @return the log-domain score for the given transition
     */
    double trans(label_id from, label_id to) const;

    /**
     * @param from The origin state
     * @param to The destination state
     * @return the score for the given transition
     */
    double trans_exp(label_id from, label_id to) const;

    /**
     * @param time The time step
     * @param lbl The state
     * @return the forward score for the given state at the given time
     */
    double forward(uint64_t time, label_id lbl) const;

    /**
     * @param time The time step
     * @param lbl The state
     * @return the backward score for the given state at the given time
     */
    double backward(uint64_t time, label_id lbl) const;

    /**
     * @param time The time step
     * @param lbl The state
     * @return the marginal probability of being in the given state at
     * the given time
     */
    double state_marginal(uint64_t time, label_id lbl) const;

    /**
     * @param from The origin state
     * @param to The destination state
     * @return the marginal probability of transitioning from the
     * origin state to the destination state
     */
    double trans_marginal(label_id from, label_id to) const;

    /**
     * Computes the loss function for a given sequence.
     *
     * @param seq The reference sequence
     * @return the value of the loss function with respect to this
     * sequence
     */
    double loss(const sequence& seq) const;

  private:
    /**
     * Computes the transition marginals.
     */
    void transition_marginals();

    /**
     * Computes the state marginals.
     */
    void state_marginals();

    /// Stores the state scores in log-domain
    double_matrix state_;
    /// Stores the state scores
    double_matrix state_exp_;
    /// Stores the transition scores in log-domain
    double_matrix trans_;
    /// Stores the transition scores
    double_matrix trans_exp_;

    /// Stores the forward trellis, if computed
    util::optional<forward_trellis> fwd_;
    /// Stores the backward trellis, if computed
    util::optional<trellis> bwd_;
    /// Stores the state marginals, if computed
    util::optional<double_matrix> state_mrg_;
    /// Stores the transition marginals, if computed
    util::optional<double_matrix> trans_mrg_;
};

}
}

#endif
