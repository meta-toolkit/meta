/**
 * @file sgd.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_SGD_H_
#define META_CLASSIFY_SGD_H_

#include "meta/classify/binary_classifier_factory.h"
#include "meta/classify/classifier/binary_classifier.h"
#include "meta/classify/classifier/online_binary_classifier.h"
#include "meta/learn/loss/loss_function.h"
#include "meta/learn/sgd.h"
#include "meta/util/disk_vector.h"
#include "meta/meta.h"

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
 * learning-rate = 0.5
 * convergence-threshold = 1e-3
 * l2-regularization = 1e-7
 * l1-regularization = 0
 * max-iter = 5
 * calibrate = false
 * ~~~
 */
class sgd : public online_binary_classifier
{
  public:
    /// The default \f$\gamma\f$ parameter.
    const static constexpr double default_gamma = 1e-3;

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
        learn::sgd_model::options_type options, double gamma = default_gamma,
        size_t max_iter = default_max_iter, bool calibrate = false);

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
