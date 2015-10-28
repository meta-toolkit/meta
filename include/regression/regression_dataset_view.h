/**
 * @file binary_dataset_view.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_REGRESSION_DATASET_VIEW_H_
#define META_REGRESSION_DATASET_VIEW_H_

#include "learn/dataset_view.h"

namespace meta
{
namespace regression
{

using regression_dataset = learn::labeled_dataset<double>;

class regression_dataset_view : public learn::dataset_view
{
  public:
    regression_dataset_view(const regression_dataset& dset) : dataset_view{dset}
    {
        // nothing
    }

    template <class RandomEngine>
    regression_dataset_view(const regression_dataset& dset, RandomEngine&& rng)
        : dataset_view{dset, std::forward<RandomEngine>(rng)}
    {
        // nothing
    }

    regression_dataset_view(const regression_dataset_view& rdv, iterator begin,
                            iterator end)
        : dataset_view{rdv, begin, end}
    {
        // nothing
    }

    double label(const instance_type& instance) const
    {
        return dset<regression_dataset>().label(instance);
    }
};
}
}
#endif
