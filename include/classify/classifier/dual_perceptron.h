/**
 * @file dual_perceptron.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_DUAL_PERCEPTRON_H_
#define META_CLASSIFY_DUAL_PERCEPTRON_H_

#include "classify/classifier_factory.h"
#include "classify/classifier/classifier.h"
#include "classify/kernel/polynomial.h"
#include "util/functional.h"
#include "meta.h"

namespace meta
{
namespace classify
{

/**
 * Implements a perceptron classifier, but using the dual formulation of
 * the problem. This allows the perceptron to be used for data that is not
 * necessarily linearly separable via the use of a kernel function.
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
    const static std::string id;

    /**
     * Constructs a dual_perceptron classifier over the given index
     * and with the given paramters.
     *
     * @param idx The index to run the classifier on
     * @param kernel_fn The kernel function to be used
     * @param alpha \f$\alpha\f$, the learning rate
     * @param gamma \f$\gamma\f$, the error threshold (in terms of
     *  percentage of mistakes on one training run)
     * @param bias \f$b\f$, the bias
     * @param max_iter The maximum allowed iterations for training.
     */
    template <class Kernel>
    dual_perceptron(std::shared_ptr<index::forward_index> idx,
                    Kernel&& kernel_fn = kernel::polynomial{},
                    double alpha = default_alpha, double gamma = default_gamma,
                    double bias = default_bias,
                    uint64_t max_iter = default_max_iter)
        : classifier{std::move(idx)},
          alpha_{alpha},
          gamma_{gamma},
          bias_{bias},
          max_iter_{max_iter}
    {
        std::function<double(pdata, pdata)> fun
            = [=](const pdata& a, const pdata& b) {
                return kernel_fn(a, b);
            };
        kernel_ = functional::memoize(fun);
    }

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
    void train(const std::vector<doc_id>& docs) override;

    /**
     * Classifies the given document.
     * The class label returned is
     * \f$\arg\!\max_k(\sum_d(w_k^d*(K(d,x) + b))\f$---in other words, the
     * class whose associated weight vector gives the highest result.
     *
     * @param doc The document to be classified
     * @return the class label determined for the document
     */
    class_label classify(doc_id d_id) override;

    /**
     * Resets all learned information for this perceptron so it may be
     * re-learned.
     */
    void reset() override;

  private:
    /**
     * Decreases the "weight" (mistake count) for a given class label
     * and document.
     *
     * @param label The class label
     * @param id The document
     */
    void decrease_weight(const class_label& label, const doc_id& id);

    /**
     * The "weight" (mistake count) vectors for each class label.
     */
    std::unordered_map<class_label, std::unordered_map<doc_id, uint64_t>>
        weights_;

    /**
     * Convenience typedef for the postings data type.
     */
    using pdata = decltype(idx_->search_primary(doc_id{}));

    /**
     * The kernel function to be used in lieu of a dot product.
     */
    std::function<double(pdata, pdata)> kernel_;

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
std::unique_ptr<classifier> make_classifier<dual_perceptron>(
        const cpptoml::table&, std::shared_ptr<index::forward_index>);
}
}
#endif
