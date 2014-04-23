/**
 * @file crf.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_CRF_H_
#define META_SEQUENCE_CRF_H_

#include "sequence/observation.h"
#include "sequence/sequence.h"
#include "sequence/analyzers/sequence_analyzer.h"
#include "sequence/trellis.h"
#include "util/dense_matrix.h"
#include "util/disk_vector.h"
#include "util/optional.h"
#include "util/range.h"

namespace meta
{
namespace sequence
{

MAKE_NUMERIC_IDENTIFIER(crf_feature_id, uint64_t)

/**
 * Linear-chain conditional random field for POS tagging and chunking
 * applications. Learned using l2 regularized stochastic gradient descent.
 *
 * This CRF implementation uses node-observation features only. This means
 * that feature templates look like \f$f(o_t, s_t)\f$ and
 * \f$f(s_{t-1}, s_t)\f$ only. This is done for memory efficiency and to
 * avoid overfitting.
 *
 * @see http://homepages.inf.ed.ac.uk/csutton/publications/crftut-fnt.pdf
 */
class crf
{
  public:
    /**
     * Wrapper to represent the parameters used during learning. The
     * defaults are sane, and so most users should simply initialize the
     * default parameter object when training the crf.
     */
    struct parameters
    {
        /**
         * The regularization parameter.
         */
        double c2 = 1;

        /**
         * The convergence threshold. Once the difference in the loss
         * between `period` iterations is less than this value, learning
         * will stop.
         */
        double delta = 1e-5;

        /**
         * The period used to check for convergence.
         */
        uint64_t period = 10;

        /**
         * The transformed regularization parameter. (This is set by the
         * CRF internally based on `c2` and the training set size.)
         */
        double lambda = 0;

        /**
         * The offset for the learning rate. The learning rate follows a
         * the following schedule:
         *
         * \f$\eta = \frac{1}{\lambda * (t_0 + t)}\f$
         *
         * where \f$t\f$ is the number of examples seen thus far.
         */
        double t0 = 0;

        /**
         * The maximum number of iterations to allow the gradient descent
         * to run for.
         */
        uint64_t max_iters = 1000;

        /**
         * The initial starting value for \f$\eta\f$, the learning rate,
         * during calibration.
         */
        double calibration_eta = 0.1;

        /**
         * The rate at which to adjust \f$\eta\f$ during calibration.
         */
        double calibration_rate = 2.0;

        /**
         * The maximum number of samples to use during calibration.
         */
        uint64_t calibration_samples = 1000;

        /**
         * The maximum number of candidate \f$\eta\f$-s to consider during
         * calibration.
         */
        uint64_t calibration_trials = 10;
    };

    /**
     * Interface for tagging. The tagger itself is not thread safe, but
     * individual threads that wish to perform tagging operations can make
     * a thread-local tagger.
     */
    class tagger;

    /**
     * Constructs a new CRF, storing model parameters in the given prefix.
     * If a crf model already exists in the given prefix, it will be
     * loaded; otherwise, the directory will be created.
     *
     * @param prefix The prefix (folder) to load/store model files
     */
    crf(const std::string& prefix);

    /**
     * Trains a new CRF model on the given examples. The examples are
     * assumed to have been run through a `sequence_analyzer` first to
     * generate features for every observation in every sequence.
     *
     * @param params The parameters for the learning algorithm
     * @param examples The labeled training examples
     * @return the loss for the last epoch during training
     */
    double train(parameters params, const std::vector<sequence>& examples);

    /**
     * Constructs a new tagging interface that references the current
     * model.
     *
     * @return a new tagging interface for this model
     */
    tagger make_tagger() const;

    /**
     * @return the number of labels possible under this model.
     */
    uint64_t num_labels() const;

  private:

    /**
     * A dense_matrix of doubles, used frequently in training and testing
     * for holding score information under the model.
     */
    using double_matrix = util::dense_matrix<double>;

    /**
     * A range representing a set of feature functions (ids).
     */
    using feature_range = util::basic_range<crf_feature_id>;

    /**
     * Internal class that holds scoring information for sequences under
     * the current model.
     */
    class scorer
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
    // grant the scorer access to the model weights
    friend scorer;

    /**
     * Scorer for performing viterbi-based tagging.
     */
    class viterbi_scorer
    {
      public:
        /**
         * Constructs a new scorer against the given model.
         * @param model The model to score with
         */
        viterbi_scorer(const crf& model);

        /**
         * Runs the viterbi algorithm to produce a trellis with
         * back-pointers.
         *
         * @param seq The sequence to score
         * @return a trellis with back-pointers indicating the path with
         * the highest score
         */
        viterbi_trellis viterbi(const sequence& seq);
      private:
        /// the internal scorer used
        scorer scorer_;
        /// a back-pointer to the model this scorer uses to tag
        const crf* model_;
    };

  public:
    class tagger
    {
      public:
        /**
         * Constructs a tagger against the given model.
         * @param model The model to use for the tagging
         */
        tagger(const crf& model);

        /**
         * Tags a sequence. The tags will be filled in on the `label` field
         * of each observation within the sequence. (You will need to ask
         * your analyzer for what the human-readable `tag_t` is for each
         * `label_id` yourself.)
         * @param seq The sequence to be tagged.
         */
        void tag(sequence& seq);
      private:
        /// the scorer used internally to run viterbi
        viterbi_scorer scorer_;
        /// the number of labels
        uint64_t num_labels_;
    };

  private:
    /**
     * Initializes the CRF model based on the set of training examples.
     * This function runs the "feature generation" portion of the training,
     * where we try to find all state-observation and
     * transition-observation functions that are active in the training
     * data.
     *
     * @param examples The training examples
     */
    void initialize(const std::vector<sequence>& examples);

    /**
     * Loads the CRF model from the files stored on disk.
     */
    void load_model();

    /**
     * Completely resets the model weights.
     */
    void reset();

    /**
     * Determines a good initial setting for the learning rate. Based on
     * Leon Bottou's SGD implementation.
     *
     * @param params The parameters for learning
     * @param indices The vector of shuffled indices for the random
     * sampling
     * @param examples The (unshuffled) training examples
     * @return The optimal `t0` found by calibration, which determines the
     * initial learning rate \f$\eta\f$.
     */
    double calibrate(parameters params, const std::vector<uint64_t>& indices,
                     const std::vector<sequence>& examples);

    /**
     * @param idx The internal crf model feature id
     * @return a const reference to the weight associated with this feature
     */
    const double& obs_weight(crf_feature_id idx) const;

    /**
     * @param idx The internal crf model feature id
     * @return a reference to the weight associated with this feature
     */
    double& obs_weight(crf_feature_id idx);

    /**
     * @param idx The internal crf model feature id
     * @return a const reference to the weight associated with this feature
     */
    const double& trans_weight(crf_feature_id idx) const;

    /**
     * @param idx The internal crf model feature id
     * @return a reference to the weight associated with this feature
     */
    double& trans_weight(crf_feature_id idx);

    /**
     * @param fid The external observation feature id
     * @return a range of internal crf model feature ids for state
     * features that are active for this observation
     */
    feature_range obs_range(feature_id fid) const;

    /**
     * @param lbl The label
     * @return a range of internal crf model feature ids for transitions
     * that are active for this state
     */
    feature_range trans_range(label_id lbl) const;

    /**
     * @param idx The internal crf model feature id
     * @return the label associated with this state-based feature id
     */
    label_id observation(crf_feature_id idx) const;

    /**
     * @param idx The internal crf model feature id
     * @return the destination label associated with this transition
     * feature id
     */
    label_id transition(crf_feature_id idx) const;

    /**
     * Performs a single epoch of training.
     *
     * @param params The learning parameters
     * @param progress The progress logger to use
     * @param iter The current epoch
     * @param indices The shuffled indices for the random sampling
     * @param examples The (not shuffled) training examples
     * @param scorer The scorer to re-use
     * @return the loss for this training epoch
     */
    double epoch(parameters params, printing::progress& progress,
                 uint64_t iter, const std::vector<uint64_t>& indices,
                 const std::vector<sequence>& examples, scorer& scorer);

    /**
     * Performs a single iteration within a training epoch.
     *
     * @param params The learning parameters
     * @param iter The current number of total iterations (\f$t\f$)
     * @param seq The sequence to use to update model parameters
     * @param scorer The scorer to re-use
     * @return the loss associated with this single iteration within the
     * epoch
     */
    double iteration(parameters params, uint64_t iter, const sequence& seq,
                     scorer& scorer);

    /**
     * Updates the model parameters based on the observation expectation
     * part of the gradient.
     *
     * @param seq The sequence to use
     * @param gain The amount to scale the weight updates by
     */
    void gradient_observation_expectation(const sequence& seq, double gain);

    /**
     * Updates the model parameters based on the model expectation part of
     * the gradient.
     *
     * @param seq The sequence to use
     * @param gain The amount to scale the weight updates by
     * @param scr The scorer to re-use for computing the marginal
     * probabilities
     */
    void gradient_model_expectation(const sequence& seq, double gain,
                                    const scorer& scr);

    /**
     * @return the current l2 norm of the weights (\f$w^T w\f$)
     */
    double l2norm() const;

    /**
     * Updates all of the weights by re-scaling by the current scale
     * parameter, and sets the scale parameter to 1 after doing so.
     */
    void rescale();

    /**
     * Represents the feature id range for a given observation:
     * `observation_ranges_[i]` gives the start of a range of
     * crf_feature_ids (indexing into the `observation_weights_`) that have
     * fired for feature_id `i`, and `observation_ranges_[i + 1]`
     * gives the end of the range. (If `i` is the end, then the size of
     * `observation_weights_` gives the last id.)
     */
    util::optional<util::disk_vector<crf_feature_id>> observation_ranges_;

    /**
     * Analogous to the observation range, but for transitions.
     * `transition_ranges_[i]` gives the start of a range of feature_ids
     * (indexing into `transition_weights_`) that have fired for label_id
     * `i`, and `transition_ranges_[i+1]` gives the end of the range. (If
     * `i` is the end, then the size of `transition_weights_` gives the
     * last id.)
     */
    util::optional<util::disk_vector<crf_feature_id>> transition_ranges_;

    /**
     * Represents the state that fired for a given observation
     * feature. This is a parallel vector with `observation_weights_`,
     * where `observations_[f]` gives the label_id for the observation
     * feature `f`.
     */
    util::optional<util::disk_vector<label_id>> observations_;

    /**
     * Represents the destination label for a given transition feature.
     * This is a parallel vector with `transition_weights_`, where
     * `transitions_[f]` gives the destination for transition feature `f`.
     */
    util::optional<util::disk_vector<label_id>> transitions_;

    /**
     * The weights for all of the node-observation features. Indexes must
     * be taken from the `observation_ranges_` vector.
     */
    util::optional<util::disk_vector<double>> observation_weights_;

    /**
     * Weights for all of the transition features. Indexes must be taken
     * from the `transition_ranges_` vector.
     */
    util::optional<util::disk_vector<double>> transition_weights_;

    /// the current decay factor applied to all of the weights
    double scale_;
    /// the number of allowed labels
    uint64_t num_labels_;
    /// the prefix (folder) where model files are to be stored
    const std::string& prefix_;
};
}
}
#endif
