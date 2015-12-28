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

#include "meta/classify/classifier_factory.h"
#include "meta/classify/classifier/classifier.h"
#include "meta/hashing/hash.h"
#include "meta/index/forward_index.h"
#include "meta/meta.h"

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
 *
 * Required config parameters:
 * ~~~toml
 * [classifier]
 * method = "svm-wrapper"
 * path = "path-to-libsvm-modules"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [classifier]
 * kernel = "quadratic" # or "none", "cubic", "quartic", "rbf", or "sigmoid"
 * ~~~
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
     * @param docs The training documents
     * @param svm_path The path to the liblinear/libsvm library
     * @param kernel_opt Which kind of kernel you want to use (default:
     * None)
     */
    svm_wrapper(dataset_view_type docs, const std::string& svm_path,
                kernel kernel_opt = kernel::None);

    /**
     * Loads a svm_wrapper from a stream.
     * @param in The stream to read from
     */
    svm_wrapper(std::istream& in);

    void save(std::ostream& out) const override;

    /**
     * Classifies a document into a specific group, as determined by
     * training data.
     * @param doc The document to classify
     * @return the class it belongs to
     */
    class_label classify(const feature_vector& doc) const override;

    /**
     * Classifies a collection document into specific groups, as determined
     * by training data.
     *
     * @param docs The documents to classify
     * @return a confusion_matrix detailing the performance of the
     * classifier
     */
    confusion_matrix test(dataset_view_type docs) const override;

    /**
     * The identifier for this classifier.
     */
    const static util::string_view id;

  private:
    /** the path to the liblinear/libsvm library */
    const std::string svm_path_;

    /** keeps track of which arguments are necessary for which kernel
     * function */
    const static std::unordered_map<kernel, std::string, hashing::hash<>>
        options_;

    /** which kernel function to use for this SVM */
    kernel kernel_;

    /** used to select which executable to use (libsvm or liblinear) */
    std::string executable_;

    /** the list of class_labels (mainly for serializing the model) */
    std::vector<class_label> labels_;
};

/**
 * Specialization of the factory method used for creating svm_wrapper
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
make_classifier<svm_wrapper>(const cpptoml::table&,
                             multiclass_dataset_view training);
}
}

#endif
