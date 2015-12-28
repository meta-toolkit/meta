/**
 * @file online_classifier.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONLINE_CLASSIFIER_H_
#define META_CLASSIFY_ONLINE_CLASSIFIER_H_

#include "meta/classify/classifier/classifier.h"

namespace meta
{
namespace classify
{

/**
 * A multi-class classifier that can be updated in-place with new
 * documents either in batches or one-at-a-time.
 */
class online_classifier : public classifier
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
    virtual void train_one(const feature_vector& doc, const class_label& label)
        = 0;
};
}
}
#endif
