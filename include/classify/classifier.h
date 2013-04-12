/**
 * @file classifier.h
 */

#ifndef _CLASSIFIER_H_
#define _CLASSIFIER_H_

#include <string>
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
        virtual std::string classify(const index::Document & doc) const = 0;

        /**
         * Classifies a document into a specific group, as determined by
         * training data.
         * @param doc The document to classify
         * @return the class it belongs to
         */
        ConfusionMatrix classify_all(const std::vector<index::Document> & docs) const;

        /**
         * Performs k-fold cross-validation on a set of training documents.
         * @param test_docs Testing documents
         * @param k The number of folds
         * @return a ConfusionMatrix containing the results over all the folds
         */
        ConfusionMatrix cross_validate(const std::vector<index::Document> & test_docs, size_t k) const;
};

}
}

#endif
