/**
 * @file one_vs_all.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONE_VS_ALL_H_
#define META_CLASSIFY_ONE_VS_ALL_H_

#include "meta/classify/classifier/binary_classifier.h"
#include "meta/classify/classifier_factory.h"
#include "meta/classify/classifier/online_classifier.h"
#include "meta/meta.h"

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
     * Constructs a new one_vs_all classifier on the given index by using
     * the given function to create binary_classifiers. The `create`
     * parameter must take a single parameter: the positive label for the
     * binary classifier to be created.
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
