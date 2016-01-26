/**
 * @file sgd.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_REGRESSION_SGD_H_
#define META_REGRESSION_SGD_H_

#include "meta/learn/sgd.h"
#include "meta/regression/models/regressor.h"
#include "meta/regression/regressor_factory.h"

namespace meta
{
namespace regression
{

/**
 * Implements stochastic gradient descent for learning regression models.
 *
 * Required config parameters:
 * ~~~toml
 * [regressor]
 * method = "sgd"
 * loss = "least-squares" # or huber
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [regressor]
 * learning-rate = 0.5
 * convergence-threshold = 1e-3
 * l2-regularization = 1e-7
 * l1-regularization = 0
 * max-iter = 5
 * calibrate = true
 * ~~~
 */
class sgd : public regressor
{
  public:
    /// The default convergence threshold
    const static constexpr double default_gamma = 1e-3;

    /// The default number of allowed iterations
    const static constexpr size_t default_max_iter = 5;

    /**
     * @param docs The training documents
     * @param loss The loss function
     * @param options The options for the SGD learner
     * @param gamma The convergence threshold
     * @param max_iter The maximum allowed iterations
     */
    sgd(dataset_view_type docs,
        std::unique_ptr<learn::loss::loss_function> loss,
        learn::sgd_model::options_type options, double gamma = default_gamma,
        size_t max_iter = default_max_iter, bool calibrate = true);

    /**
     * Loads an sgd regressor from a stream.
     * @param in The stream to read from
     */
    sgd(std::istream& in);

    void save(std::ostream& out) const override;

    /**
     * Trains the model on an additional set of documents.
     * @param docs The document collection to update the model with
     */
    void train(dataset_view_type docs);

    /**
     * Trains the model on a single instance.
     * @param doc The document to learn from
     * @param label The actual label of the document
     */
    void train_one(const feature_vector& doc, double label);

    double predict(const feature_vector& doc) const override;

    /**
     * The identifier for this regressor
     */
    const static util::string_view id;

  private:
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
 * Specialization of the factory method used to create sgd regressors.
 */
template <>
std::unique_ptr<regressor>
make_regressor<sgd>(const cpptoml::table& config,
                    regression_dataset_view training);
}
}
#endif
