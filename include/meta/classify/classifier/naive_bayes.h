/**
 * @file naive_bayes.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NAIVE_BAYES_H_
#define META_NAIVE_BAYES_H_

#include <unordered_map>
#include "meta/index/forward_index.h"
#include "meta/classify/classifier/classifier.h"
#include "meta/classify/classifier_factory.h"
#include "meta/meta.h"
#include "meta/stats/multinomial.h"
#include "meta/util/sparse_vector.h"

#include "meta/classify/multiclass_dataset_view.h"

namespace meta
{
namespace classify
{

/**
 * Implements the Naive Bayes classifier, a simplistic probabilistic classifier
 * that uses Bayes' theorem with strong feature independence assumptions.
 *
 * Required config parameters: none.
 * Optional config parameters:
 * ~~~toml
 * [classifier]
 * method = "naive-bayes"
 * alpha = 0.1
 * beta = 0.1
 * ~~~
 */
class naive_bayes : public classifier
{
  public:
    /// The default \f$\alpha\f$ parameter.
    const static constexpr double default_alpha = 0.1;
    /// The default \f$beta\f$ parameter.
    const static constexpr double default_beta = 0.1;

    /**
     * Constructor: learns class models based on a collection of training
     * documents. Training calculates \f$P(term|class)\f$ and
     * \f$P(class)\f$ for all the training documents.
     * @param docs The training data
     * @param alpha Optional smoothing parameter for term frequencies
     * @param beta Optional smoothing parameter for class frequencies
     */
    naive_bayes(dataset_view_type docs, double alpha = default_alpha,
                double beta = default_beta);

    /**
     * Constructor: loads a pre-trained model from an input stream.
     * @param in The input stream to load from
     */
    naive_bayes(std::istream& in);

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     * @param instance The document to classify
     * @return the class it belongs to
     */
    class_label classify(const feature_vector& instance) const override;

    /**
     * Saves the model to a stream.
     * @param os The stream to save to
     */
    void save(std::ostream& os) const override;

    /**
     * The identifier for this classifier.
     */
    const static util::string_view id;

  private:

    void train(const dataset_view_type& docs);

    /**
     * Contains P(term|class) for each class.
     */
    util::sparse_vector<class_label, stats::multinomial<term_id>> term_probs_;

    /**
     * Contains the number of documents in each class
     */
    stats::multinomial<class_label> class_probs_;
};

class naive_bayes_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * Specialization of the factory method used for creating naive bayes
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<naive_bayes>(const cpptoml::table& config,
                                 multiclass_dataset_view training);
}
}
#endif
