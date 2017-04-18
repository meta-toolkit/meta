/**
 * @file one_vs_all.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONE_VS_ALL_H_
#define META_CLASSIFY_ONE_VS_ALL_H_

#include "meta/meta.h"

#include "meta/classify/classifier/binary_classifier.h"
#include "meta/classify/classifier/online_classifier.h"
#include "meta/classify/classifier_factory.h"
#include "meta/parallel/parallel_for.h"
#include "meta/util/traits.h"

namespace meta
{
namespace classify
{

/**
 * Generalizes binary classifiers to operate over multiclass types using the
 * one vs all method.
 *
 * Required config parameters:
 * ~~~toml
 * [classifier]
 * method = "one-vs-all"
 * [classifier.base]
 * method = "sgd" # for example
 * loss = "hinge" # for example
 * prefix = "sgd-model" # for example
 * ~~~
 */
class one_vs_all : public online_classifier
{
  public:
    /**
     * Constructs a new one_vs_all classifier over the given training set
     * by using the creation function specified for creating
     * binary_classifiers. The creation function shall take one parameter
     * (a binary_dataset_view for training) and shall return a unique_ptr
     * to a binary_classifer.
     *
     * @param docs The training data
     * @param creator The function to create and train binary_classifiers
     */
    template <class Creator,
              class = typename std::
                  enable_if<util::is_callable<Creator>::value>::type>
    one_vs_all(multiclass_dataset_view docs, Creator&& creator)
    {
        classifiers_.reserve(docs.total_labels());
        for (auto it = docs.labels_begin(), end = docs.labels_end(); it != end;
             ++it)
            classifiers_[it->first] = nullptr;

        parallel::parallel_for(
            classifiers_.begin(), classifiers_.end(),
            [&](std::pair<const class_label,
                          std::unique_ptr<binary_classifier>>& pr) {
                binary_dataset_view bdv{
                    docs, [&](const instance_type& instance) {
                        return docs.label(instance) == pr.first;
                    }};
                pr.second = creator(bdv);
            });
    }

    /**
     * Constructs a new one_vs_all classifier on the given training set by
     * using the given configuration for creating binary_classifiers
     * through the make_binary_classifier factory function.
     *
     * @param docs The training data
     * @param base The configuration for the individual binary_classifiers
     */
    one_vs_all(multiclass_dataset_view docs, const cpptoml::table& base);

    /**
     * Loads a one_vs_all classifier from a stream.
     * @param in The stream to read from
     */
    one_vs_all(std::istream& in);

    void save(std::ostream& out) const override;

    class_label classify(const feature_vector& doc) const override;

    void train(dataset_view_type docs) override;

    void train_one(const feature_vector& doc,
                   const class_label& label) override;

    /**
     * The identifier for this classifier.
     */
    const static util::string_view id;

  private:
    /**
     * The set of classifiers this ensemble uses for classification.
     */
    std::unordered_map<class_label, std::unique_ptr<binary_classifier>>
        classifiers_;
};

/**
 * Specialization of the factory method used to create one_vs_all
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
make_classifier<one_vs_all>(const cpptoml::table&,
                            multiclass_dataset_view training);
}
}
#endif
