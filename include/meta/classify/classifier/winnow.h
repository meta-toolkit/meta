/**
 * @file winnow.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_WINNOW_H_
#define META_WINNOW_H_

#include <vector>
#include <unordered_map>
#include "meta/index/forward_index.h"
#include "meta/classify/classifier/classifier.h"
#include "meta/classify/classifier_factory.h"
#include "meta/meta.h"

namespace meta
{
namespace classify
{

/**
 * Implements the Winnow classifier, a simplistic linear classifier for
 * linearly-separable data. As opposed to winnow (which uses an additive
 * update rule), winnow uses a multiplicative update rule.
 *
 * Required config parameters: none
 *
 * Optional config parameters:
 * ~~~toml
 * [classifier]
 * method = "winnow"
 * m = 1.5
 * gamma = 0.05
 * max-iter = 100
 * ~~~
 */
class winnow : public classifier
{
  public:
    /// The default \f$m\f$ parameter.
    const static constexpr double default_m = 1.5;
    /// The default \f$gamma\f$ parameter.
    const static constexpr double default_gamma = 0.05;
    /// The default number of allowed iterations.
    const static constexpr size_t default_max_iter = 100;

    /**
     * Constructs a winnow classifier with the given multiplier,
     * error threshold, and maximum iterations.
     *
     * This constructor trains the winnow on the given training documents.
     * Maintains a set of weight vectors \f$w_1,\ldots,w_K\f$ where \f$K\f$
     * is the number of classes and updates them for each training document
     * seen in each iteration. This continues until the error threshold is
     * met or the maximum number of iterations is completed.
     *
     * @param docs The training documents
     * @param m \f$m\f$, the multiplicative learning rate
     * @param gamma \f$gamma\f$, the error threshold
     * @param max_iter The maximum number of iterations for training
     */
    winnow(multiclass_dataset_view docs, double m = default_m,
           double gamma = default_gamma, size_t max_iter = default_max_iter);

    /**
     * Loads a winnow classifier from a stream.
     * @param in The stream to read from
     */
    winnow(std::istream& in);

    void save(std::ostream& out) const override;

    /**
     * Classifies the given document.
     * The class label returned is
     * \f$\argmax_k(w_k^\intercal x_n + b)\f$---in other words, the
     * class whose associated weight vector gives the highest result.
     *
     * @param doc The document to be classified
     * @return the class label determined for the document
     */
    class_label classify(const feature_vector& doc) const override;

    /**
     * The identifier for this classifier.
     */
    const static util::string_view id;

  private:
    /**
     * @return the given term's weight in the weight vector for the given
     * class
     *
     * @param label The class label for the weight vector we want
     * @param term The term whose weight should be returned
     */
    double get_weight(const class_label& label, const term_id& term) const;

    /**
     * Initializes the weight vectors to zero for every class label.
     *
     * @param docs The set of documents to collect class labels from.
     */
    void zero_weights(const multiclass_dataset_view& docs);

    /**
     * The weight vectors for each class label.
     */
    std::unordered_map<class_label, std::unordered_map<term_id, double>>
        weights_;

    /// \f$m\f$, the multiplicative learning rate.
    const double m_;

    /// \f$\gamma\f$, the error threshold.
    const double gamma_;

    /// The maximum number of iterations for training.
    const size_t max_iter_;
};

/**
 * Specialization of the factory method used for creating winnow
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<winnow>(const cpptoml::table& config,
                            multiclass_dataset_view training);
}
}
#endif
