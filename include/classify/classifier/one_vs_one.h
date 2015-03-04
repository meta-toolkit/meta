/**
 * @file one_vs_one.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONE_VS_ONE_H_
#define META_CLASSIFY_ONE_VS_ONE_H_

#include "classify/classifier/binary_classifier.h"
#include "classify/classifier_factory.h"
#include "meta.h"

namespace meta
{
namespace classify
{

/**
 * Ensemble method adaptor for extending binary_classifiers to the
 * multi-class classification case by using a one-vs-one strategy. This
 * entails creating a classifier for each pair of classes, and assigning
 * the label which gets the most "votes" from each individual
 * binary_classifier as the label for a given document.
 */
class one_vs_one : public classifier
{
  public:
    /**
     * Constructs a new one_vs_one classifier using the given
     * forward_index to retrieve document information and using the given
     * function to create individual binary_classifiers for each pair of
     * classes.
     *
     * @param idx The forward_index to retrieve documents from
     * @param create A function to create binary_classifiers: should
     * return a unique_ptr to a binary_classifier when given two class
     * labels as parameters
     */
    template <class Function>
    one_vs_one(std::shared_ptr<index::forward_index> idx, Function&& create)
        : classifier{std::move(idx)}
    {
        auto labels = idx_->class_labels();
        for (uint64_t i = 0; i < labels.size(); ++i)
        {
            for (uint64_t j = i + 1; j < labels.size(); ++j)
            {
                classifiers_.emplace_back(create(labels[i], labels[j]));
            }
        }
    }

    void train(const std::vector<doc_id>& docs) override;

    class_label classify(doc_id d_id) override;

    void reset() override;

    /**
     * The identifier for this classifier.
     */
    const static std::string id;

  private:
    /// the set of classifiers used in the ensemble
    std::vector<std::unique_ptr<binary_classifier>> classifiers_;
};

/**
 * Specialization of the factory method used to create one_vs_all
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<one_vs_one>(const cpptoml::table&,
                                std::shared_ptr<index::forward_index>);
}
}
#endif
