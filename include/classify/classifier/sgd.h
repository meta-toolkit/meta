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
#include "classify/classifier/online_binary_classifier.h"
#include "learn/loss/loss_function.h"
#include "learn/sgd.h"
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
 *
 * Required config parameters:
 * ~~~toml
 * [classifier]
 * method = "sgd"
 * prefix = "path-to-model"
 * loss = "hinge" # or "huber", "least-squares", "logistic", etc
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [classifier]
 * alpha = 0.5
 * gamma = 1e-6
 * lambda = 0.0001
 * max-iter = 50
 * ~~~
 */
class sgd : public online_binary_classifier
{
  public:
    /// The default \f$\alpha\f$ parameter.
    const static constexpr double default_alpha = 0.5;
    /// The default \f$\gamma\f$ parameter.
    const static constexpr double default_gamma = 1e-3;
    /// The default \f$\lambda\f$ parameter.
    const static constexpr double default_lambda = 1e-7;
    /// The default number of allowed iterations.
    const static constexpr size_t default_max_iter = 5;

    /**
     * @param docs The training documents
     * @param alpha \f$alpha\f$, the learning rate
     * @param gamma \f$gamma\f$, the error threshold
     * @param bias \f$b\f$, the bias
     * @param lambda \f$\lambda\f$, the regularization constant
     * @param max_iter The maximum number of iterations for training.
     */
    sgd(binary_dataset_view docs,
        std::unique_ptr<learn::loss::loss_function> loss,
        double alpha = default_alpha, double gamma = default_gamma,
        double lambda = default_lambda, size_t max_iter = default_max_iter);

    /**
     * Loads an sgd classifier from a stream.
     * @param in The stream to read from
     */
    sgd(std::istream& in);

    void save(std::ostream& out) const override;

    void train(binary_dataset_view docs) override;

    void train_one(const feature_vector& doc, bool label) override;

    /**
     * Returns the dot product with the current weight vector. Used
     * mainly for generalization of a binary decision problem to a
     * multiclass decision problem.
     *
     * @param doc The document to compute the dot product with
     * @return the dot product with the current weight vector
     */
    double predict(const feature_vector& doc) const override;

    /**
     * The identifier for this classifier.
     */
    const static util::string_view id;

  private:
    /**
     * Internal version of train_one that returns the loss.
     */
    double train_instance(const feature_vector& doc, bool label);

    /// The model
    learn::sgd_model model_;

    /// \f$\gamma\f$, the error threshold.
    const double gamma_;

    /// The maximum number of iterations for training.
    const size_t max_iter_;

    /// The loss function to be used for the update.
    std::unique_ptr<learn::loss::loss_function> loss_;
};

/**
 * Specialization of the factory method used to create sgd classifiers.
 */
template <>
std::unique_ptr<binary_classifier>
    make_binary_classifier<sgd>(const cpptoml::table& config,
                                binary_dataset_view training);
}
}
#endif
