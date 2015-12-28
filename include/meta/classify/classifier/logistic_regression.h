/**
 * @file logistic_regression.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LOGISTIC_REGRESSION_H_
#define META_LOGISTIC_REGRESSION_H_

#include "meta/classify/classifier_factory.h"
#include "meta/classify/classifier/classifier.h"
#include "meta/classify/classifier/sgd.h"
#include "meta/index/forward_index.h"
#include "meta/meta.h"
#include "meta/util/optional.h"

namespace meta
{
namespace classify
{

/**
 * Multinomial logistic regression. If there are \f$K\f$ classes, this uses
 * SGD to perform \f$K-1\f$ independent logistic regressions by picking
 * class \f$K\f$ as a pivot (that is, each of the \f$K-1\f$ independent
 * regressions is done against the \f$K\f$-th class).
 *
 * The probability of each class is then:
 *
  \f{align*}{
    P(y_i = 1) &=
      \frac{\exp(predict_1(x_i))}{1+\sum_{k=1}^K \exp(predict_k(x_i))}\\
    P(y_i = 2) &=
      \frac{\exp(predict_2(x_i))}{1+\sum_{k=1}^K \exp(predict_k(x_i))}\\
    &\vdots\\
    P(y_i = K-1) &=
      \frac{\exp(predict_{K-1}(x_i))}{1+\sum_{k=1}^K \exp(predict_k(x_i))}\\
    P(y_i = K) &=
      \frac{1}{1+\sum_{k=1}^K \exp(predict_k(x_i))}
  \f}
 *
 * where \f$predict_k(x_i)\f$ is the result of running the `predict`
 * function on the \f$k\f$-th classifier with the \f$i\f$-th example. The
 * output of `classifier::classify()`, then, is the class with the highest
 * probability based on the above formulas.
 *
 * The individual class probabilities may be recovered by using the
 * `predict` function: this returns an `unordered_map` of `class_label` to
 * probability.
 *
 * Required config parameters:
 * ~~~toml
 * [classifier]
 * method = "logistic-regression"
 * prefix = "path-to-model"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [classifier]
 * alpha = 0.001
 * gamma = 1e-6
 * bias = 1.0
 * lambda = 0.0001
 * max-iter = 50
 * ~~~
 */
class logistic_regression : public classifier
{
  public:
    /**
     * @param docs The training data
     * @param options The options for training the sub sgd models
     * @param gamma \f$\gamma\f$, the error threshold for each of the
     * independent regressions
     * @param max_iter The maximum number of iterations for training each
     * independent regression
     */
    logistic_regression(multiclass_dataset_view docs,
                        learn::sgd_model::options_type options,
                        double gamma = sgd::default_gamma,
                        uint64_t max_iter = sgd::default_max_iter);

    /**
     * Loads a logistic_regression classifier from a stream.
     * @param in The stream to read from
     */
    logistic_regression(std::istream& in);

    void save(std::ostream& out) const override;

    /**
     * Obtains the probability that the given document belongs to each
     * class.
     *
     * @param d_id The document to obtain class-membership probabilities for
     * @return a map from class label to probability of membership
     */
    std::unordered_map<class_label, double>
        predict(const feature_vector& doc) const;

    class_label classify(const feature_vector& doc) const override;

    /// the identifier for this classifier
    const static util::string_view id;

  private:
    /// the set of \f$K-1\f$ independent classifiers
    std::unordered_map<class_label, std::unique_ptr<binary_classifier>>
        classifiers_;
    /// the class chosen to be the pivot element
    class_label pivot_;
};

/**
 * Specialization of the factory method used for creating
 * logistic_regression classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<logistic_regression>(const cpptoml::table&,
                                         multiclass_dataset_view training);
}
}
#endif
