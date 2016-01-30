/**
 * @file linear_model.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_MODEL_LINEAR_MODEL_H_
#define META_CLASSIFY_MODEL_LINEAR_MODEL_H_

#include <istream>
#include <ostream>
#include <unordered_map>

#include "meta/meta.h"
#include "meta/util/sparse_vector.h"

namespace meta
{
namespace classify
{

/**
 * Exception thrown during interactions with linear_models.
 */
class linear_model_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * A storage class for multiclass linear classifier models. This class
 * should be used to store classifiers that fit in memory.
 */
template <class FeatureId, class FeatureValue, class ClassId>
class linear_model
{
  public:
    /**
     * The identifier for features.
     */
    using feature_id = FeatureId;

    /**
     * The value type for features. Typically this is a double or a float,
     * but it could in principle be any numeric type.
     */
    using feature_value = FeatureValue;

    /**
     * The ids for the classes.
     */
    using class_id = ClassId;

    /**
     * A single weight vector for a specific feature id. This stores the
     * weights that a given feature has for each class.
     */
    using weight_vector = util::sparse_vector<class_id, feature_value>;

    /**
     * A collection of weight_vector by feature id.
     */
    using weight_vectors = std::unordered_map<feature_id, weight_vector>;

    /**
     * A class_id with an associated score.
     */
    using scored_class = std::pair<class_id, feature_value>;

    /**
     * A vector of class ids and associated scores.
     */
    using scored_classes = std::vector<scored_class>;

    /**
     * The exception thrown during interactions with linear_models.
     */
    using exception = linear_model_exception;

    /**
     * Loads a linear model from a stream. If the stream is from a file, we
     * are assuming that it has been opened in binary mode.
     *
     * @param is The stream to read from
     */
    void load(std::istream& is);

    /**
     * Writes a linear model to a stream. If the stream is from a file,w e
     * are assuming it has been opened in binary mode.
     *
     * @param os The stream to write to
     */
    void save(std::ostream& os) const;

    /**
     * Determines the highest scoring class that satisfies a filter
     * predicate for a given feature vector. Any class id that does not
     * return true when passed to the filter will not be considered. If no
     * classes satisfy the filter predicate, the default class id will be
     * returned.
     *
     * @param features The feature vector to score
     * @param filter The filter predicate to use
     * @return the highest scoring class that passes the filter predicate
     */
    template <class FeatureVector, class Filter>
    class_id best_class(FeatureVector&& features, Filter&& filter) const;

    /**
     * Determines the highest scoring class for a given feature vector.
     *
     * @param features The feature vector to score
     * @return the highest scoring class
     */
    template <class FeatureVector>
    class_id best_class(FeatureVector&& features) const;

    /**
     * Determines the top \f$k\f$ classes for a given feature vector that
     * satisfy the filter predicate. Any class id that does not return true
     * when passed to the filter will not be considered. If no classes
     * satisfy the filter predicate, the returned vector will be empty.
     *
     * @param features The feature vector to score
     * @param num The number of classes to return (\f$k\f$)
     * @param filter The filter predicate
     * @return a vector of (up to) \f$k\f$ scored classes sorted by score
     */
    template <class FeatureVector, class Filter>
    scored_classes best_classes(FeatureVector&& features, uint64_t num,
                                Filter&& filter) const;

    /**
     * Determines the top \f$k\f$ classes for a given feature vector.
     *
     * @param features The feature vector to score
     * @param num The number of classes to return (\f$k\f$)
     * @return a vector of (up to) \f$k\f$ scored classes sorted by score
     */
    template <class FeatureVector>
    scored_classes best_classes(FeatureVector&& features, uint64_t num) const;

    /**
     * Updates all of the weights of this model by adding in the
     * contribution from another set of weight vectors, multiplied by a
     * scalar value (default 1).
     *
     * @param updates The weight vector containing the updates
     * @param scale The scalar to multiply everything in the update by
     * before adding it to the model
     */
    void update(const weight_vectors& updates, feature_value scale = 1);

    /**
     * Updates a single weight in the model.
     *
     * @param cid The class
     * @param fid The feature
     * @param delta The amount to be added to that weight
     */
    void update(const class_id& cid, const feature_id& fid,
                feature_value delta);

    /**
     * Removes all weights that have value 0.
     *
     * @param log Whether or not to log statistics about the condensed
     * weight vector (info level)
     */
    void condense(bool log = false);

    /**
     * @return the weights
     */
    const weight_vectors& weights() const;

  private:
    /**
     * The weights for the model
     */
    weight_vectors weights_;
};
}
}

#include "meta/classify/models/linear_model.tcc"
#endif
