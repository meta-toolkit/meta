/**
 * @file nearest_centroid.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NEAREST_CENTROID_H_
#define META_NEAREST_CENTROID_H_

#include "index/inverted_index.h"
#include "index/forward_index.h"
#include "classify/classifier_factory.h"
#include "classify/classifier/classifier.h"

namespace meta
{
namespace classify
{

/**
 * Implements the nearest centroid classification algorithm. nearest_centroid
 * creates a prototype document for each distinct class as an average of all
 * documents in that class. This is called the centroid. A query (testing
 * document) is then compared against each centroid. The class label of the
 * centroid they query is closest to is returned.
 * @see Centroid-Based Document Classification: Analysis and Experimental
 * Results, Eui-Hong Han and George Karypis, 2000
 */
class nearest_centroid : public classifier
{
  public:
    /// Identifier for this classifier.
    const static std::string id;

    /**
     * @param idx The index to run the classifier on
     */
    nearest_centroid(std::shared_ptr<index::inverted_index> idx,
                     std::shared_ptr<index::forward_index> f_idx);

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
     * @param d_id
     * @param centroid
     * @return the cosine similarity between the query and a centroid
     */
    double cosine_sim(const std::vector<std::pair<term_id, double>>& doc,
                      const std::unordered_map<term_id, double>& centroid);

    /// Inverted index used for ranking
    std::shared_ptr<index::inverted_index> inv_idx_;

    /// The document centroids for this learner
    std::unordered_map<class_label, std::unordered_map<term_id, double>>
        centroids_;

  public:
    /**
     * Basic exception for nearest_centroid interactions.
     */
    class nearest_centroid_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};

/**
 * Specialization of the factory method used to create nearest_centroid
 * classifiers.
 */
template <>
std::unique_ptr<classifier> make_multi_index_classifier<nearest_centroid>(
    const cpptoml::table&, std::shared_ptr<index::forward_index>,
    std::shared_ptr<index::inverted_index>);
}
}
#endif
