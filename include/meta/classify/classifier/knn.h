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
#include "meta/index/inverted_index.h"
#include "meta/index/forward_index.h"
#include "meta/index/ranker/ranker.h"
#include "meta/classify/classifier_factory.h"
#include "meta/classify/classifier/classifier.h"

namespace meta
{
namespace classify
{

/**
 * Implements the k-Nearest Neighbor lazy learning classification algorithm.
 *
 * Required config parameters:
 * ~~~toml
 * [classifier]
 * method = "knn"
 * k = 10
 *
 * [classifier.ranker]
 * method = "dirichlet-prior" # any ranker id
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [classifier]
 * weighted = true # default is false
 * ~~~
 */
class knn : public classifier
{
  public:
    /**
     * Identifier for this classifier.
     */
    const static util::string_view id;

    /**
     * @param docs The training documents
     * @param idx The index to run the classifier on
     * @param k The value of k in k-NN
     * @param ranker The ranker to be used internally
     * @param weighted Whether to weight the neighbors by distance to the query
     */
    knn(multiclass_dataset_view docs,
        std::shared_ptr<index::inverted_index> idx, uint16_t k,
        std::unique_ptr<index::ranker> ranker, bool weighted = false);

    /**
     * Loads a knn classifier from a stream.
     * @param in The stream to read from
     */
    knn(std::istream& in);

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     * @param d_id The document to classify
     * @return the class it belongs to
     */
    class_label classify(const feature_vector& instance) const override;

    void save(std::ostream& out) const override;

  private:
    /**
     * @param scored
     * @param sorted
     * @return the best label
     */
    class_label select_best_label(
        const std::vector<index::search_result>& scored,
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
};

/**
 * Basic exception for knn interactions.
 */
class knn_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * Specialization of the factory method used to create knn classifiers.
 */
template <>
std::unique_ptr<classifier> make_multi_index_classifier<knn>(
    const cpptoml::table& config, multiclass_dataset_view training,
    std::shared_ptr<index::inverted_index> inv_idx);
}
}
#endif
