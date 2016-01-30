/**
 * @file binary_classifier.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_BINARY_CLASSIFIER_H_
#define META_BINARY_CLASSIFIER_H_

#include <ostream>
#include "meta/classify/binary_dataset_view.h"

namespace meta
{
namespace classify
{

/**
 * A classifier which classifies documents as "positive" or "negative".
 */
class binary_classifier
{
  public:
    using dataset_view_type = binary_dataset_view;
    using instance_type = dataset_view_type::instance_type;
    using feature_vector = learn::feature_vector;

    /**
     * Default destructor is virtual for polymorphic delete.
     */
    virtual ~binary_classifier() = default;

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     *
     * @param instance The instance to classify
     * @return the class it belongs to (true if positive, false if
     * negative)
     */
    bool classify(const feature_vector& instance) const;

    /**
     * Returns the confidence of a positive example. It should be >= 0 for
     * positive examples, < 0 for negative examples.
     *
     * @param instance The instance to classify
     * @return the "confidence" that this document is a positive example
     */
    virtual double predict(const feature_vector& instance) const = 0;

    /**
     * Saves the classifier model to a stream.
     * @param out The stream to write to
     */
    virtual void save(std::ostream& out) const = 0;
};
}
}
#endif
