/**
 * @file featurizer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_ANALYZERS_FEATURIZER_H_
#define META_ANALYZERS_FEATURIZER_H_

#include <stdexcept>

#include "meta/config.h"
#include "meta/hashing/probe_map.h"
#include "meta/util/likely.h"
#include "meta/util/shim.h"

namespace meta
{
namespace analyzers
{

/**
 * Basic exception for featurizer interactions.
 */
class featurizer_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

template <class T>
using feature_map = hashing::probe_map<std::string, T>;

/**
 * Used by analyzers to increment feature values in feature_maps
 * generically. This class type-erases a specific map class so that the
 * interface remains the same to enable virtual functions in analyzers.
 */
class featurizer
{
  public:
    /**
     * Constructs a featurizer that writes to a specific feature_map.
     */
    template <class T>
    featurizer(feature_map<T>& map) : map_{make_unique<concrete_map<T>>(map)}
    {
        static_assert(std::is_same<T, uint64_t>::value
                          || std::is_same<T, double>::value,
                      "feature map must map to uint64_t or double");
    }

    /**
     * Observes the given feature occurring val times.
     * @param feat The feature identifier
     * @param val The feature value
     */
    template <class T>
    void operator()(const std::string& feat, T val)
    {
        static_assert(std::is_integral<T>::value
                          || std::is_floating_point<T>::value,
                      "feature map must map to uint64_t or double");

        if (std::is_floating_point<T>::value)
            map_->increment(feat, static_cast<double>(val));
        else
            map_->increment(feat, static_cast<uint64_t>(val));
    }

  private:
    class map_concept
    {
      public:
        virtual void increment(const std::string& feat, double val) = 0;
        virtual void increment(const std::string& feat, uint64_t val) = 0;
        virtual ~map_concept() = default;
    };

    template <class T>
    class concrete_map : public map_concept
    {
      public:
        concrete_map(feature_map<T>& map) : map_(map)
        {
            // nothing
        }

        void increment(const std::string& feat, double val) override
        {
            if (META_UNLIKELY((!std::is_same<T, double>::value)))
                throw featurizer_exception{
                    "cannot increment double value on integer featurizer"};
            map_[feat] += val;
        }

        void increment(const std::string& feat, uint64_t val) override
        {
            map_[feat] += val;
        }

      private:
        feature_map<T>& map_;
    };

    std::unique_ptr<map_concept> map_;
};
}
}
#endif
