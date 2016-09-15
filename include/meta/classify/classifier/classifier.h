/**
 * @file classifier.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFIER_H_
#define META_CLASSIFIER_H_

#include <ostream>
#include <vector>
#include "meta/classify/confusion_matrix.h"
#include "meta/index/forward_index.h"
#include "meta/learn/dataset.h"
#include "meta/classify/multiclass_dataset_view.h"

namespace meta
{
namespace classify
{

/**
 * A classifier uses a document's feature space to identify which group it
 * belongs to. This is a base class that defines an interface for
 * multi-class classification.
 */
class classifier
{
  public:
    using instance_type = multiclass_dataset::instance_type;
    using feature_vector = learn::feature_vector;
    using dataset_view_type = multiclass_dataset_view;

    /**
     * Default destructor is virtual for polymorphic delete.
     */
    virtual ~classifier() = default;

    /**
     * Classifies an instance_type into a specific group, as determined by
     * training data.
     * @param instance The instance to classify
     * @return the class it belongs to
     */
    virtual class_label classify(const feature_vector& instance) const = 0;

    /**
     * Classifies a collection document into specific groups, as determined
     * by training data; this function will make repeated calls to
     * classify().
     * @param docs The documents to classify
     * @return a confusion_matrix detailing the performance of the
     * classifier
     */
    virtual confusion_matrix test(dataset_view_type docs) const;

    /**
     * Saves the classifier model to the output stream.
     * @param out The stream to write the model to
     */
    virtual void save(std::ostream& out) const = 0;
};

/**
 * Exception thrown from classifier operations.
 */
class classifier_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * Performs k-fold cross-validation on a set of documents.
 *
 * @param creator A function to create classifiers given a
 * multiclass_dataset_view
 * @param docs Testing documents
 * @param k The number of folds
 * @param even_split Whether to evenly split the data by class for a fair
 * baseline
 * @return a confusion_matrix containing the results over all the folds
 */
template <class Creator>
confusion_matrix cross_validate(Creator&& creator,
                                classifier::dataset_view_type docs, size_t k,
                                bool even_split = false)
{
    using diff_type = decltype(docs.begin())::difference_type;

    if (even_split)
        docs = docs.create_even_split();

    // docs might be ordered by class, so make sure things are shuffled
    docs.shuffle();

    confusion_matrix matrix;
    auto step_size = docs.size() / k;
    for (size_t i = 0; i < k; ++i)
    {
        LOG(info) << "Cross-validating fold " << (i + 1) << "/" << k << ENDLG;
        multiclass_dataset_view train_view{
            docs, docs.begin() + static_cast<diff_type>(step_size), docs.end()};

        auto cls = creator(train_view);
        multiclass_dataset_view test_view{
            docs, docs.begin(),
            docs.begin() + static_cast<diff_type>(step_size)};
        auto m = cls->test(test_view);
        matrix += m;
        docs.rotate(step_size);
    }

    return matrix;
}

/**
 * Performs k-fold cross-validation on a set of documents.
 *
 * @param config The configuration to use to create the classifier
 * @param docs Testing documents
 * @param k The number of folds
 * @param even_split Whether to evenly split the data by class for a fair
 * baseline
 * @return a confusion_matrix containing the results over all the folds
 */
confusion_matrix cross_validate(const cpptoml::table& config,
                                classifier::dataset_view_type docs, size_t k,
                                bool even_split = false);
}
}
#endif
