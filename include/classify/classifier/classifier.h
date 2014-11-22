/**
 * @file classifier.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFIER_H_
#define META_CLASSIFIER_H_

#include <vector>
#include "classify/confusion_matrix.h"

namespace meta
{
namespace classify
{

/**
 * A classifier uses a document's feature space to identify which group it
 * belongs to.
 */
class classifier
{
  public:
    /**
     * @param idx The index to run the classifier on
     */
    classifier(std::shared_ptr<index::forward_index> idx);

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     * @param d_id The document to classify
     * @return the class it belongs to
     */
    virtual class_label classify(doc_id d_id) = 0;

    /**
     * Creates a classification model based on training documents.
     * @param docs The training documents
     */
    virtual void train(const std::vector<doc_id>& docs) = 0;

    /**
     * Classifies a collection document into specific groups, as determined
     * by training data; this function will make repeated calls to
     * classify().
     * @param docs The documents to classify
     * @return a confusion_matrix detailing the performance of the
     * classifier
     */
    virtual confusion_matrix test(const std::vector<doc_id>& docs);

    /**
     * Performs k-fold cross-validation on a set of documents. When using
     * this function, it is not necessary to call train() or test() first.
     * @param input_docs Testing documents
     * @param k The number of folds
     * @param even_split Whether to evenly split the data by class for a fair
     * baseline
     * @param seed The seed for the RNG used to shuffle the documents
     * @return a confusion_matrix containing the results over all the folds
     */
    virtual confusion_matrix
        cross_validate(const std::vector<doc_id>& input_docs, size_t k,
                       bool even_split = false, int seed = 1);

    /**
     * Clears any learning data associated with this classifier.
     */
    virtual void reset() = 0;

  protected:
    /** the index that the classifer is run on */
    std::shared_ptr<index::forward_index> idx_;

  private:
    /**
     * Modifies input_docs to be a vector of size <= the original vector size
     * with an even distribution of class labels per document
     * @param input_docs
     * @param seed The seed for the RNG used to shuffle the documents
     */
    void create_even_split(std::vector<doc_id>& docs, int seed = 2) const;
};
}
}
#endif
