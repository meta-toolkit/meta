/**
 * @file online_binary_classifier.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONLINE_BINARY_CLASSIFIER_H_
#define META_CLASSIFY_ONLINE_BINARY_CLASSIFIER_H_

#include "meta/classify/classifier/binary_classifier.h"

namespace meta
{
namespace classify
{

/**
 * A binary classifier that can be updated in-place with new documents,
 * either in batches or one-at-a-time.
 */
class online_binary_classifier : public binary_classifier
{
  public:
    /**
     * Updates an existing model in an online fashion on a mini-batch of
     * documents. This method is allowed to iterate over the data more than
     * once: for more fine-grained control over the amount of data
     * ingested, use train_one().
     *
     * @param docs A mini-batch to train with
     */
    virtual void train(dataset_view_type docs) = 0;

    /**
     * Updates an existing model in an online fashion on a single document.
     *
     * @param doc A single document to update with
     * @param label The expected label for the document
     */
    virtual void train_one(const feature_vector& doc, bool label) = 0;
};
}
}
#endif
