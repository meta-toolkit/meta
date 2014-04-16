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
#include "util/disk_vector.h"

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
        double lr = 0.25;
        double delta = 1e-5;
        double lambda = 0.0001;
        uint64_t period = 10;
        uint64_t max_iters = 1000;
    };

    crf(const std::string& prefix);

    double train(parameters params, const std::vector<sequence>& examples);

    void tag(sequence& seq);

    void reset();

  private:

    struct feature_range
    {
        const crf_feature_id start;
        const crf_feature_id end;
    };
    void initialize(const std::vector<sequence>& examples);

    label_id label(tag_t tag);

    const double& obs_weight(crf_feature_id idx) const;
    double& obs_weight(crf_feature_id idx);

    const double& trans_weight(crf_feature_id idx) const;
    double& trans_weight(crf_feature_id idx);

    feature_range obs_range(feature_id fid) const;
    feature_range trans_range(label_id lbl) const;

    label_id observation(crf_feature_id idx) const;
    label_id transition(crf_feature_id idx) const;

    double epoch(parameters params, printing::progress& progress,
                 const std::vector<uint64_t>& indices,
                 const std::vector<sequence>& examples);

    double iteration(parameters params, const sequence& seq);

    using double_matrix = std::vector<std::vector<double>>;

    void gradient_observation_expectation(const sequence& seq, double gain);
    void gradient_model_expectation(const sequence& seq, double gain,
                                    const double_matrix& state_mrg,
                                    const double_matrix& trans_mrg);


    void state_scores(const sequence& seq);

    void transition_scores();

    forward_trellis forward(const sequence& seq) const;

    trellis backward(const sequence& seq, const forward_trellis& fwd) const;

    double_matrix state_marginals(const forward_trellis& fwd,
                                  const trellis& bwd) const;

    double_matrix transition_marginals(const forward_trellis& fwd,
                                       const trellis& bwd) const;

    double loss(const sequence& seq, const forward_trellis& fwd);

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

    /// The label_id mapping (tag_t to label_id)
    util::invertible_map<tag_t, label_id> label_id_mapping_;

    double scale_;

    double_matrix state_;
    double_matrix trans_;

    const std::string& prefix_;
};
}
}
#endif
