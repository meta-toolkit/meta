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
#include "index/forward_index.h"
#include "classify/classifier/classifier.h"
#include "classify/classifier_factory.h"
#include "meta.h"
#include "stats/multinomial.h"
#include "util/sparse_vector.h"

namespace meta
{
namespace classify
{

/**
 * Implements the Naive Bayes classifier, a simplistic probabilistic classifier
 * that uses Bayes' theorem with strong feature independence assumptions.
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
     * documents.
     * @param idx The index to run the classifier on
     * @param alpha Optional smoothing parameter for term frequencies
     * @param beta Optional smoothing parameter for class frequencies
     */
    naive_bayes(std::shared_ptr<index::forward_index> idx,
                double alpha = default_alpha, double beta = default_beta);

    /**
     * Creates a classification model based on training documents.
     * Calculates \f$P(term|class)\f$ and \f$P(class)\f$ for all the
     * training documents.
     * @param docs The training documents
     */
    void train(const std::vector<doc_id>& docs) override;

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     * @param d_id The document to classify
     * @return the class it belongs to
     */
    class_label classify(doc_id d_id) override;

    /**
     * Resets any learning information associated with this classifier.
     */
    void reset() override;

    /**
     * The identifier for this classifier.
     */
    const static std::string id;

  private:
    /**
     * Contains P(term|class) for each class.
     */
    util::sparse_vector<class_label, stats::multinomial<term_id>> term_probs_;

    /**
     * Contains the number of documents in each class
     */
    stats::multinomial<class_label> class_probs_;
};

/**
 * Specialization of the factory method used for creating naive bayes
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<naive_bayes>(const cpptoml::table& config,
                                 std::shared_ptr<index::forward_index> idx);

}
}
#endif
