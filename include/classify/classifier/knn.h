/**
 * @file knn.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_KNN_H_
#define META_KNN_H_

#include <unordered_set>
#include "index/inverted_index.h"
#include "index/forward_index.h"
#include "index/ranker/ranker.h"
#include "classify/classifier_factory.h"
#include "classify/classifier/classifier.h"

namespace meta
{
namespace classify
{

/**
 * Implements the k-Nearest Neighbor lazy learning classification algorithm.
 */
class knn : public classifier
{
  public:
    /**
     * Identifier for this classifier.
     */
    const static std::string id;

    /**
     * @param idx The index to run the classifier on
     * @param ranker The ranker to be used internally
     * @param k The value of k in k-NN
     * @param args Arguments to the chosen ranker constructor
     * @param weighted Whether to weight the neighbors by distance to the query
     */
    knn(std::shared_ptr<index::inverted_index> idx,
        std::shared_ptr<index::forward_index> f_idx, uint16_t k,
        std::unique_ptr<index::ranker> ranker, bool weighted = false);

    /**
     * Creates a classification model based on training documents.
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

  private:
    /**
     * @param scored
     * @param sorted
     * @return the best label
     */
    class_label select_best_label(
        const std::vector<std::pair<doc_id, double>>& scored,
        const std::vector<std::pair<class_label, uint16_t>>& sorted) const;

    /** the inverted index used for ranking */
    std::shared_ptr<index::inverted_index> inv_idx_;

    /** the value of k in k-NN */
    uint16_t k_;

    /**
     * The ranker that is used to score the queries in the index.
     */
    std::unique_ptr<index::ranker> ranker_;

    /** documents that are "legal" to be used in the results */
    std::unordered_set<doc_id> legal_docs_;

    /** Whether we want the neighbors to be weighted by distance or not */
    const bool weighted_;

  public:
    /**
     * Basic exception for knn interactions.
     */
    class knn_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};

/**
 * Specialization of the factory method used to create knn classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_multi_index_classifier<knn>(const cpptoml::table&,
                                     std::shared_ptr<index::forward_index>,
                                     std::shared_ptr<index::inverted_index>);
}
}
#endif
