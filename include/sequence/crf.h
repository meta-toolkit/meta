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
        double lr = 0.001;
        double delta = 1e-5;
        double lambda = 0.0001;
        uint64_t period = 10;
        uint64_t max_iters = 1000;
    };

    crf(const std::string& prefix);
    crf(sequence_analyzer& analyzer);

    double train(parameters params, const std::vector<sequence>& examples);

    void tag(sequence& seq);

    void reset();

  private:
    /// weights are scaled by scale_ automatically
    const double& weight(label_id, feature_id) const;
    double& weight(label_id, feature_id);

    const double& weight(label_id, label_id) const;
    double& weight(label_id, label_id);

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

    double loss(const sequence& seq, const forward_trellis& fwd) const;

    double l2norm() const;

    /**
     * The weights for node-observation features, indexed as
     * `observation_weights_[l*M + i]` where `l` is a label_id, `i` is
     * a feature_id, and `M` is the number of unique feature_ids
     */
    util::disk_vector<double> observation_weights_;

    /**
     * The weights for transition features, indexed as
     * `transition_weights_[i*L + j]` where `i` and `j` are label_ids and
     * `L` is the number of labels
     */
    util::disk_vector<double> transition_weights_;

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
