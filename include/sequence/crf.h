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
    struct parameters
    {
        double c2 = 1;
        double delta = 1e-5;
        double lambda = 0;
        double t0 = 0;
        uint64_t period = 10;
        uint64_t max_iters = 1000;
        double calibration_eta = 0.1;
        double calibration_rate = 2.0;
        uint64_t calibration_samples = 1000;
        uint64_t calibration_trials = 10;
    };

    crf(const std::string& prefix);

    double train(parameters params, const std::vector<sequence>& examples);

    void tag(sequence& seq);

    void reset();

    uint64_t num_labels() const;

  private:

    using double_matrix = util::dense_matrix<double>;
    using feature_range = util::basic_range<crf_feature_id>;

    class scorer
    {
      public:
        void score(const crf& model, const sequence& seq);

        void forward();
        void backward();
        void marginals();

        void viterbi();

        double state(uint64_t time, label_id lbl) const;
        double state_exp(uint64_t time, label_id lbl) const;
        double trans(label_id from, label_id to) const;
        double trans_exp(label_id from, label_id to) const;

        double forward(uint64_t time, label_id lbl) const;
        double backward(uint64_t time, label_id lbl) const;

        double state_marginal(uint64_t time, label_id lbl) const;
        double trans_marginal(label_id from, label_id to) const;

        double loss(const sequence& seq) const;

        class exception : public std::runtime_error
        {
            using std::runtime_error::runtime_error;
        };

      private:
        void transition_scores(const crf& model);
        void state_scores(const crf& model, const sequence& seq);

        void transition_marginals();
        void state_marginals();

        double_matrix state_;
        double_matrix state_exp_;
        double_matrix trans_;
        double_matrix trans_exp_;

        util::optional<forward_trellis> fwd_;
        util::optional<trellis> bwd_;
        util::optional<double_matrix> state_mrg_;
        util::optional<double_matrix> trans_mrg_;
    };
    friend scorer;

    void initialize(const std::vector<sequence>& examples);

    /**
     * Determines a good initial setting for the learning rate. Based on
     * Leon Bottou's SGD implementation.
     */
    double calibrate(parameters params, const std::vector<uint64_t>& indices,
                     const std::vector<sequence>& examples);

    const double& obs_weight(crf_feature_id idx) const;
    double& obs_weight(crf_feature_id idx);

    const double& trans_weight(crf_feature_id idx) const;
    double& trans_weight(crf_feature_id idx);

    feature_range obs_range(feature_id fid) const;
    feature_range trans_range(label_id lbl) const;

    label_id observation(crf_feature_id idx) const;
    label_id transition(crf_feature_id idx) const;

    double epoch(parameters params, printing::progress& progress,
                 uint64_t iter, const std::vector<uint64_t>& indices,
                 const std::vector<sequence>& examples, scorer& scorer);

    double iteration(parameters params, uint64_t iter, const sequence& seq,
                     scorer& scorer);

    void gradient_observation_expectation(const sequence& seq, double gain);
    void gradient_model_expectation(const sequence& seq, double gain,
                                    const scorer& scr);

    double l2norm() const;

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

    double scale_;
    uint64_t num_labels_;

    const std::string& prefix_;
};
}
}
#endif
