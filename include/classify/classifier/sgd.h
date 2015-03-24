/**
 * @file sgd.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_SGD_H_
#define META_CLASSIFY_SGD_H_

#include "classify/binary_classifier_factory.h"
#include "classify/classifier/binary_classifier.h"
#include "classify/loss/loss_function.h"
#include "util/disk_vector.h"
#include "meta.h"

namespace meta
{
namespace classify
{

/**
 * Implements stochastic gradient descent for learning binary linear
 * classifiers. These may be extended to multiclass classification using
 * the one_vs_all or all_vs_all adapters.
 */
class sgd : public binary_classifier
{
  public:
    /// The default \f$\alpha\f$ parameter.
    const static constexpr double default_alpha = 0.001;
    /// The default \f$\gamma\f$ parameter.
    const static constexpr double default_gamma = 1e-6;
    /// The default \f$b\f$ parameter.
    const static constexpr double default_bias = 1;
    /// The default \f$\lambda\f$ parameter.
    const static constexpr double default_lambda = 0.0001;
    /// The default number of allowed iterations.
    const static constexpr size_t default_max_iter = 50;

    /**
     * @param prefix The prefix for the model file
     * @param idx The index to run the classifier on
     * @param positive The label for the positive class (all others
     *  are assumed to be negative).
     * @param negative The label to return for negative documents.
     * @param alpha \f$alpha\f$, the learning rate
     * @param gamma \f$gamma\f$, the error threshold
     * @param bias \f$b\f$, the bias
     * @param lambda \f$\lambda\f$, the regularization constant
     * @param max_iter The maximum number of iterations for training.
     */
    sgd(const std::string& prefix, std::shared_ptr<index::forward_index> idx,
        class_label positive, class_label negative,
        std::unique_ptr<loss::loss_function> loss, double alpha = default_alpha,
        double gamma = default_gamma, double bias = default_bias,
        double lambda = default_lambda, size_t max_iter = default_max_iter);

    /**
     * Returns the dot product with the current weight vector. Used
     * mainly for generalization of a binary decision problem to a
     * multiclass decision problem.
     *
     * @param doc The document to compute the dot product with
     * @return the dot product with the current weight vector
     */
    double predict(doc_id d_id) const override;

    void train(const std::vector<doc_id>& docs) override;

    void reset() override;

    /**
     * The identifier for this classifier.
     */
    const static std::string id;

  private:
    /// The weights vector.
    util::disk_vector<double> weights_;

    /// The scalar coefficient for the weights vector.
    double coeff_{1.0};

    /// \f$\alpha\f$, the learning rate.
    const double alpha_;

    /// \f$\gamma\f$, the error threshold.
    const double gamma_;

    /// \f$b\f$, the bias.
    double bias_;

    /// The weight of the bias term for each document (defaults to 1)
    const double bias_weight_;

    /// \f$\lambda\f$, the regularization constant
    const double lambda_;

    /// The maximum number of iterations for training.
    const size_t max_iter_;

    /// The loss function to be used for the update.
    std::unique_ptr<loss::loss_function> loss_;

    /**
     * Typedef for the sparse vector training/test instances.
     */
    using counts_t = std::vector<std::pair<term_id, double>>;

    /**
     * Helper function that takes a sparse vector. Used as a
     * performance optimization during training.
     *
     * @param doc the document to form a prediction for
     * @return the dot product with the current weight vector
     */
    double predict(const counts_t& doc) const;
};

/**
 * Specialization of the factory method used to create sgd classifiers.
 */
template <>
std::unique_ptr<binary_classifier>
    make_binary_classifier<sgd>(const cpptoml::table& config,
                                std::shared_ptr<index::forward_index> idx,
                                class_label positive, class_label negative);
}
}
#endif
