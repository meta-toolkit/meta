/**
 * @file instance.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LEARN_INSTANCE_H_
#define META_LEARN_INSTANCE_H_

#include "meta/config.h"
#include "meta/util/identifiers.h"
#include "meta/util/sparse_vector.h"

namespace meta
{
namespace learn
{
using feature_id = term_id;
using feature_vector = util::sparse_vector<feature_id, double>;

MAKE_NUMERIC_IDENTIFIER_UDL(instance_id, uint64_t, _inst_id)

inline void print_liblinear(std::ostream& os, const feature_vector& weights)
{
    for (const auto& count : weights)
        os << ' ' << (count.first + 1) << ':' << count.second;
}

/**
 * Represents an instance in the dataset, consisting of its id and
 * feature_vector.
 */
struct instance
{
    template <class ForwardIterator>
    instance(instance_id inst_id, ForwardIterator begin, ForwardIterator end)
        : id{inst_id}, weights{begin, end}
    {
        // nothing
    }

    instance(instance_id inst_id, feature_vector wv)
        : id{inst_id}, weights{std::move(wv)}
    {
        // nothing
    }

    instance(instance_id inst_id) : id{inst_id}, weights{}
    {
        // nothing
    }

    void print_liblinear(std::ostream& os) const
    {
        learn::print_liblinear(os, weights);
    }

    /// the id within the dataset that contains this instance
    instance_id id;
    /// the weights of the features in this instance
    const feature_vector weights;
};
}
}
#endif
