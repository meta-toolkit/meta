/**
 * @file liblinear_svm.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _LIBLINEAR_SVM_H_
#define _LIBLINEAR_SVM_H_

#include "classify/classifier.h"
#include "meta.h"

namespace meta {
namespace classify {

/**
 * Wrapper class for liblinear (http://www.csie.ntu.edu.tw/~cjlin/liblinear/)
 * implementation of support vector machine classification.
 */
class liblinear_svm: public classifier
{
    public:

        /**
         * Constructor.
         * @param liblinear_path The path to the liblinear library
         */
        liblinear_svm(const std::string & liblinear_path);

        /**
         * Classifies a document into a specific group, as determined by
         * training data.
         * @param doc The document to classify
         * @return the class it belongs to
         */
        class_label classify(const index::document & doc);

        /**
         * Creates a classification model based on training documents.
         * @param docs The training documents
         */
        void train(const std::vector<index::document> & docs);

        /**
         * Classifies a collection document into specific groups, as determined by
         * training data; this function will make repeated calls to classify().
         * @param docs The documents to classify
         * @return a confusion_matrix detailing the performance of the classifier
         */
        confusion_matrix test(const std::vector<index::document> & docs);

        /**
         * Clears any learned data from this classifier.
         */
        void reset();

    private:

        /** the path to the liblinear library */
        const std::string _liblinear_path;

        /** keeps track of the class to integer mapping */
        util::InvertibleMap<class_label, int> _mapping;
};

}
}

#endif
