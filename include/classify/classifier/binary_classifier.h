/**
 * @file binary_classifier.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_BINARY_CLASSIFIER_H_
#define META_BINARY_CLASSIFIER_H_

#include "classify/classifier/classifier.h"

namespace meta
{
namespace classify
{

/**
 * A classifier which classifies documents as "positive" or "negative".
 * This classifier must be given the label to consider as a "positive"
 * example and the label to return for a "negative" example.
 */
class binary_classifier : public classifier
{
  public:
    /**
     * Creates a new binary classifier using the given index to retrieve
     * documents, treating anything with the given positive label as a
     * positive example and everything else as a negative example. The
     * negative class label will be returned for anything that is deemed
     * negative.
     *
     * @param idx The forward_index to read documents from
     * @param positive The class label to return for "positive" documents
     * @param negative The class label to return for "negative" documents
     */
    binary_classifier(std::shared_ptr<index::forward_index> idx,
                      class_label positive, class_label negative);

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     *
     * @param d_id The document to classify
     * @return the class it belongs to
     */
    class_label classify(doc_id d_id) final;

    /**
     * @param d_id The document to classify
     * @return the "confidence" that this document is a positive example
     */
    virtual double predict(doc_id d_id) const = 0;

    /**
     * @return the "positive" label for this classifier
     */
    const class_label& positive_label() const;

    /**
     * @return the "negative" label for this classifier
     */
    const class_label& negative_label() const;

  private:
    /**
     * The label that marks positive examples
     */
    const class_label positive_;

    /**
     * The label to return when an example is classified as "negative"
     */
    const class_label negative_;
};

}
}
#endif
