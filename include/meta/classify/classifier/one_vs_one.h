/**
 * @file one_vs_one.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONE_VS_ONE_H_
#define META_CLASSIFY_ONE_VS_ONE_H_

#include "meta/classify/classifier/binary_classifier.h"
#include "meta/classify/classifier_factory.h"
#include "meta/classify/classifier/online_classifier.h"

#include "meta/hashing/hash.h"
#include "meta/meta.h"

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
 *
 * Required config parameters:
 * ~~~toml
 * [classifier]
 * method = "one-vs-one"
 * [classifier.base]
 * method = "sgd" # for example
 * loss = "hinge" # for example
 * prefix = "sgd-model" # for example
 * ~~~
 */
class one_vs_one : public online_classifier
{
  public:
    /**
     * Constructs a new one_vs_one classifier using the given
     * forward_index to retrieve document information and using the given
     * function to create individual binary_classifiers for each pair of
     * classes.
     *
     * @param docs The documents to train with
     * @param base The configuration for the individual binary_classifiers
     */
    one_vs_one(multiclass_dataset_view docs, const cpptoml::table& base);

    /**
     * Loads a one_vs_one classifier from a stream.
     * @param in The stream to read from
     */
    one_vs_one(std::istream& in);

    void save(std::ostream& out) const override;

    class_label classify(const feature_vector& instance) const override;

    void train(dataset_view_type docs) override;

    void train_one(const feature_vector& doc,
                   const class_label& label) override;

    /**
     * The identifier for this classifier.
     */
    const static util::string_view id;

  private:
    struct problem_type
    {
        class_label positive;
        class_label negative;

        template <class HashAlgorithm>
        friend void hash_append(HashAlgorithm& h, const problem_type& prob)
        {
            hash_append(h, prob.positive);
            hash_append(h, prob.negative);
        }

        friend bool operator==(const problem_type& a, const problem_type& b)
        {
            return std::tie(a.positive, a.negative)
                   == std::tie(b.positive, b.negative);
        }

        friend bool operator!=(const problem_type& a, const problem_type& b)
        {
            return !(a == b);
        }
    };

    using classifier_map_type
        = std::unordered_map<problem_type, std::unique_ptr<binary_classifier>,
                             hashing::hash<>>;
    /**
     * the set of classifiers used in the ensemble, indexed by their
     * positive/negative class pair
     */
    classifier_map_type classifiers_;
};

/**
 * Specialization of the factory method used to create one_vs_all
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<one_vs_one>(const cpptoml::table&,
                                multiclass_dataset_view training);
}
}
#endif
