/**
 * @file sgd.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LEARN_SGD_H_
#define META_LEARN_SGD_H_

#include <vector>
#include "learn/dataset.h"
#include "learn/loss/loss_function.h"

namespace meta
{
namespace learn
{

/**
 * A generic stochastic gradient descent learner for binary classification
 * or regression. This model applies the scale-invariant, adaptive gradient
 * method (NAG) described in Ross, Mineiro, and Langford and should be well
 * suited to most datasets with little learning rate fiddling.
 *
 * It supports both L1 and L2 regularization, which may be used at the same
 * time. L2 regularization uses the "scalar-times-a-vector" trick for
 * efficient shrinking during training, and L1 regularization is performed
 * using the cumulative penalty method of Tsuruoka, Tsujii, and Ananiadou.
 *
 * @see http://arxiv.org/abs/1305.6646
 * @see http://www.aclweb.org/anthology/P09-1054
 */
class sgd_model
{
  public:
    /// The default learning rate
    const static constexpr double default_learning_rate = 0.5;

    /// The default l2 regularization parameter
    const static constexpr double default_l2_regularizer = 1e-7;

    /// The default l1 regularization parameter (defaults to off)
    const static constexpr double default_l1_regularizer = 0;

    /**
     * Construction options for the model, specifying the learning rate
     * and the regularizer values.
     */
    struct options_type
    {
        double learning_rate = default_learning_rate;
        double l2_regularizer = default_l2_regularizer;
        double l1_regularizer = default_l1_regularizer;

        options_type()
        {
            // nothing; workaround for clang not liking the default
            // initialized argument below without an explicit default
            // constructor definition, and gcc 4.8 not being happy with a
            // braced init list to set the values to work around clang...
        }
    };

    /**
     * Constructs a new model with the specified number of features,
     * learning rate, and regularization.
     */
    sgd_model(std::size_t num_features, options_type options = {});

    /**
     * Loads a model from a stream (so that one could continue training).
     */
    sgd_model(std::istream& in);

    /**
     * Saves the current model state to a stream.
     */
    void save(std::ostream& out) const;

    /**
     * Gives a prediction for an input vector. This is simply \f$w^T x\f$.
     * @return the prediction
     */
    double predict(const feature_vector& x) const;

    /**
     * Updates the model for a specific instance.
     *
     * @param x The instance to update with
     * @param expected_label The ground truth label
     * @param loss The loss function to use for the update
     *
     * @return the loss incurred for this example
     */
    double train_one(const feature_vector& x, double expected_label,
                     const loss::loss_function& loss);

  private:
    /**
     * Per-feature representation of the weight vector.
     *
     * This stores four values: the actual weight, the current scale
     * factor, and the running sum of squared gradients in order to
     * implement the normalized adaptive gradient algorithm from Ross,
     * Mineiro, and Langford, as well as the total cumulative L1 penalty to
     * implement the L1 regularization proposed by Tsuruoka, Tsujii, and
     * Ananiadou.
     */
    struct weight_type
    {
        double weight = 0;
        double scale = 0;
        double grad_squared = 0;
        double cumulative_penalty = 0;
    };

    void penalize(weight_type& weight_val);

    /// The per-feature weight information
    std::vector<weight_type> weights_;

    /// The weight information for the bias term
    weight_type bias_;

    /// The current scalar to multiply weights in the weight vector by
    double scale_;

    /// The update scale factor (\f$N\f$ in Ross et. al.)
    double update_scale_;

    /// The learning rate (\f$\eta\f$ in Ross et. al.)
    double lr_;

    /// The l2 regularization constant
    double l2_regularization_;

    /// The l1 regularization constant
    double l1_regularization_;

    /// The total number of observed examples
    std::size_t t_;
};
}
}

#endif
