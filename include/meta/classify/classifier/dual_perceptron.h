/**
 * @file dual_perceptron.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_DUAL_PERCEPTRON_H_
#define META_CLASSIFY_DUAL_PERCEPTRON_H_

#include "meta/classify/classifier_factory.h"
#include "meta/classify/classifier/classifier.h"
#include "meta/classify/kernel/polynomial.h"
#include "meta/meta.h"

namespace meta
{
namespace classify
{

/**
 * Implements a perceptron classifier, but using the dual formulation of
 * the problem. This allows the perceptron to be used for data that is not
 * necessarily linearly separable via the use of a kernel function.
 *
 * Required config parameters:
 * ~~~toml
 * [classifier]
 * method = "dual-perceptron"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [classifier]
 * alpha = 0.1
 * gamma = 0.05
 * bias = 0.0
 *
 * # kernels (optional, but if used they have required params)
 * [classifier.kernel]
 * method = "polynomial"
 * power = 1 # optional, default 1
 * c = 1 # optional, default 1
 *
 * # or
 * [classifier.kernel]
 * method = "rbf"
 * gamma = 0.1 # value required
 *
 * # or
 * [classifier.kernel]
 * method = "sigmoid"
 * alpha = 0.1 # value required
 * c = 0.1 # value required
 * ~~~
 */
class dual_perceptron : public classifier
{
  public:
    /// The default \f$\alpha\f$ parameter
    const static constexpr double default_alpha = 0.1;

    /// The default \f$\gamma\f$ parameter
    const static constexpr double default_gamma = 0.05;

    /// The default \f$b\f$ parameter
    const static constexpr double default_bias = 0;

    /// The default number of allowed iterations
    const static constexpr uint64_t default_max_iter = 100;

    /// The identifier for this classifier
    const static util::string_view id;

    /**
     * Constructs a dual_perceptron classifier over the given index
     * and with the given paramters.
     *
     * @param docs The training data
     * @param kernel_fn The kernel function to be used
     * @param alpha \f$\alpha\f$, the learning rate
     * @param gamma \f$\gamma\f$, the error threshold (in terms of
     *  percentage of mistakes on one training run)
     * @param bias \f$b\f$, the bias
     * @param max_iter The maximum allowed iterations for training.
     */
    dual_perceptron(multiclass_dataset_view docs,
                    std::unique_ptr<kernel::kernel> kernel_fn,
                    double alpha = default_alpha, double gamma = default_gamma,
                    double bias = default_bias,
                    uint64_t max_iter = default_max_iter);

    /**
     * Loads a dual_perceptron model from a stream.
     * @param in The input stream to read the model from
     */
    dual_perceptron(std::istream& in);

    /**
     * Classifies the given document.
     * The class label returned is
     * \f$\arg\!\max_k(\sum_d(w_k^d*(K(d,x) + b))\f$---in other words, the
     * class whose associated weight vector gives the highest result.
     *
     * @param instance The document to be classified
     * @return the class label determined for the document
     */
    class_label classify(const feature_vector& instance) const override;

    void save(std::ostream& out) const override;

  private:
    /**
     * Trains the perceptron on the given training documents.
     * Maintains a set of weight vectors \f$w_1,\ldots,w_K\f$ where
     * \f$K\f$ is the number of classes and updates them for each
     * training document seen in each iteration. This continues until
     * the error threshold is met or the maximum number of iterations
     * is completed.
     *
     * Contrary to the regular perceptron, since this is the dual
     * formulation, its vectors are "mistake vectors" that keep track
     * of how often a given training instance was misclassified.
     *
     * @param docs The training set
     */
    void train(multiclass_dataset_view docs);

    /**
     * Decreases the "weight" (mistake count) for a given class label
     * and document.
     *
     * @param label The class label
     * @param id The document
     */
    void decrease_weight(const class_label& label,
                         const learn::instance_id& id);

    /**
     * The "weight" (mistake count) vectors for each class label.
     */
    std::unordered_map<class_label, std::unordered_map<learn::instance_id,
                                                       uint64_t>> weights_;

    /**
     * The memorized training data vectors where mistakes were made.
     */
    util::sparse_vector<learn::instance_id, feature_vector> svs_;

    /**
     * The kernel function to be used in lieu of a dot product.
     */
    std::unique_ptr<kernel::kernel> kernel_;

    /**
     * \f$\alpha\f$, the learning rate
     */
    const double alpha_;

    /**
     * \f$\gamma\f$, the error threshold (in terms of percentage of
     * mistakes on the training data in one iteration of training).
     */
    const double gamma_;

    /**
     * \f$b\f$, the bias factor.
     */
    const double bias_;

    /**
     * The maximum number of iterations for training.
     */
    const uint64_t max_iter_;
};

/**
 * Specialization of the factory function used to create dual_perceptrons.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<dual_perceptron>(const cpptoml::table&,
                                     multiclass_dataset_view training);
}
}
#endif
