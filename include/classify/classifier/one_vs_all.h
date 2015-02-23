/**
 * @file one_vs_all.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONE_VS_ALL_H_
#define META_CLASSIFY_ONE_VS_ALL_H_

#include "classify/classifier/binary_classifier.h"
#include "classify/classifier_factory.h"
#include "meta.h"

namespace meta
{
namespace classify
{

/**
 * Generalizes binary classifiers to operate over multiclass types using the
 * one vs all method.
 */
class one_vs_all : public classifier
{
  public:
    /**
     * Constructs a new one_vs_all classifier on the given index by using
     * the given function to create binary_classifiers. The `create`
     * parameter must take a single parameter: the positive label for the
     * binary classifier to be created.
     *
     * @param idx The forward_index to be passed to each binary_classifier
     * created for the ensemble
     * @param create A Callable (function object, lambda, etc.) that is
     * used to create the individual binary_classifiers.
     */
    template <class Function>
    one_vs_all(std::shared_ptr<index::forward_index> idx, Function&& create)
        : classifier{std::move(idx)}
    {
        for (const auto& label : idx_->class_labels())
            classifiers_.emplace(label, create(label));
    }

    void train(const std::vector<doc_id>& docs) override;

    class_label classify(doc_id d_id) override;

    void reset() override;

    /**
     * The identifier for this classifier.
     */
    const static std::string id;

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
                                std::shared_ptr<index::forward_index>);
}
}
#endif
