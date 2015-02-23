/**
 * @file svm_wrapper.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SVM_WRAPPER_H_
#define META_SVM_WRAPPER_H_

#include <unordered_map>
#include "classify/classifier_factory.h"
#include "classify/classifier/classifier.h"
#include "index/forward_index.h"
#include "meta.h"

namespace meta
{
namespace classify
{

/**
 * Wrapper class for liblinear (http://www.csie.ntu.edu.tw/~cjlin/liblinear/)
 * and libsvm (http://www.csie.ntu.edu.tw/~cjlin/libsvm/)
 * implementation of support vector machine classification.
 *
 * To use this class, make sure that you have checked out the libsvm-modules
 * submodule and have compiled both libsvm and liblinear.
 *
 * If no kernel is selected, liblinear is used. Otherwise, libsvm is used.
 */
class svm_wrapper : public classifier
{
  public:
    /**
     * Selects which kernel to use. "None" uses liblinear. Any other kernel
     * uses libsvm.
     */
    enum kernel
    {
        None,
        Quadratic,
        Cubic,
        Quartic,
        RBF,
        Sigmoid
    };

    /**
     * Constructor.
     * @param idx The index to run the classifier on
     * @param svm_path The path to the liblinear/libsvm library
     * @param kernel_opt Which kind of kernel you want to use (default:
     * None)
     */
    svm_wrapper(std::shared_ptr<index::forward_index> idx,
                const std::string& svm_path, kernel kernel_opt = kernel::None);

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     * @param doc The document to classify
     * @return the class it belongs to
     */
    class_label classify(doc_id d_id) override;

    /**
     * Creates a classification model based on training documents.
     * @param docs The training documents
     */
    void train(const std::vector<doc_id>& docs) override;

    /**
     * Classifies a collection document into specific groups, as determined
     * by training data; this function will make repeated calls to
     * classify().
     * @param docs The documents to classify
     * @return a confusion_matrix detailing the performance of the
     * classifier
     */
    confusion_matrix test(const std::vector<doc_id>& docs) override;

    /**
     * Clears any learned data from this classifier.
     */
    void reset() override;

    /**
     * The identifier for this classifier.
     */
    const static std::string id;

  private:
    /** the path to the liblinear/libsvm library */
    const std::string svm_path_;

    /** keeps track of which arguments are necessary for which kernel
     * function */
    const static std::unordered_map<kernel, std::string, std::hash<int>>
        options_;

    /** which kernel function to use for this SVM */
    kernel kernel_;

    /** used to select which executable to use (libsvm or liblinear) */
    std::string executable_;
};

/**
 * Specialization of the factory method used for creating svm_wrapper
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<svm_wrapper>(const cpptoml::table&,
                                 std::shared_ptr<index::forward_index>);
}
}

#endif
