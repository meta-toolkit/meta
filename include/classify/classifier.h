/**
 * @file classifier.h
 */

#ifndef _CLASSIFIER_H_
#define _CLASSIFIER_H_

#include <vector>
#include "index/document.h"
#include "classify/confusion_matrix.h"

namespace meta {
namespace classify {

/**
 * A classifier uses a document's feature space to identify which group it
 * belongs to.
 */
class classifier
{
    public:

        /**
         * Classifies a document into a specific group, as determined by
         * training data.
         * @param doc The document to classify
         * @return the class it belongs to
         */
        virtual class_label classify(const index::Document & doc) const = 0;

        /**
         * Creates a classification model based on training documents.
         * @param docs The training documents
         */
        virtual void train(const std::vector<index::Document> & docs) = 0;

        /**
         * Classifies a collection document into specific groups, as determined by
         * training data; this function will make repeated calls to classify().
         * @param docs The documents to classify
         * @return a confusion_matrix detailing the performance of the classifier
         */
        confusion_matrix test(const std::vector<index::Document> & docs) const;

        /**
         * Performs k-fold cross-validation on a set of documents. When using
         * this function, it is not necessary to call train() or test() first.
         * @param docs Testing documents
         * @param k The number of folds
         * @return a confusion_matrix containing the results over all the folds
         */
        confusion_matrix cross_validate(const std::vector<index::Document> & docs, size_t k) const;
};

}
}

#endif
