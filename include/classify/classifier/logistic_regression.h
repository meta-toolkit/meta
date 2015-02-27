/**
 * @file logistic_regression.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LOGISTIC_REGRESSION_H_
#define META_LOGISTIC_REGRESSION_H_

#include "classify/classifier_factory.h"
#include "classify/classifier/classifier.h"
#include "classify/classifier/sgd.h"
#include "index/forward_index.h"
#include "meta.h"

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
 */
class logistic_regression : public classifier
{
  public:
    /**
     * @param prefix The prefix for the model files
     * @param idx The index to run the classifier on
     * @param alpha \f$\alpha\f$, the learning rate for each of the
     * independent regressions
     * @param gamma \f$\gamma\f$, the error threshold for each of the
     * independent regressions
     * @param bias \f$b\f$, the bias term for each of the independent
     * regressions
     * @param lambda \f$\lambda\f$, the regularization constant for each
     * of the independent regressions
     * @param max_iter The maximum number of iterations for training each
     * independent regression
     */
    logistic_regression(const std::string& prefix,
                        std::shared_ptr<index::forward_index> idx,
                        double alpha = sgd::default_alpha,
                        double gamma = sgd::default_gamma,
                        double bias = sgd::default_bias,
                        double lambda = sgd::default_lambda,
                        uint64_t max_iter = sgd::default_max_iter);

    /**
     * Obtains the probability that the given document belongs to each
     * class.
     *
     * @param d_id The document to obtain class-membership probabilities for
     * @return a map from class label to probability of membership
     */
    std::unordered_map<class_label, double> predict(doc_id d_id);

    virtual class_label classify(doc_id d_id) override;

    virtual void train(const std::vector<doc_id>& docs) override;

    virtual void reset() override;

    /// the identifier for this classifier
    const static std::string id;

  private:
    /// the set of \f$K-1\f$ independent classifiers
    std::unordered_map<class_label, sgd> classifiers_;
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
                                         std::shared_ptr<index::forward_index>);
}
}
#endif
