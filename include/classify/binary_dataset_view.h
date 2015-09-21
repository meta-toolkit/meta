/**
 * @file binary_dataset_view.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_BINARY_DATASET_VIEW_H_
#define META_CLASSIFY_BINARY_DATASET_VIEW_H_

#include "learn/dataset_view.h"
#include "classify/multiclass_dataset_view.h"

namespace meta
{
namespace classify
{

using binary_dataset = learn::labeled_dataset<bool>;

/**
 * A non-owning view of a dataset with binary class albels. This is
 * designed to be used with binary classifiers.
 */
class binary_dataset_view : public learn::dataset_view
{
  public:
    binary_dataset_view(const binary_dataset& dset)
        : dataset_view{dset},
          label_fn_{[this](const instance_type& instance)
                    {
                        return this->dset<binary_dataset>().label(instance);
                    }}
    {
        // nothing
    }

    template <class RandomEngine>
    binary_dataset_view(const binary_dataset& dset, RandomEngine&& rng)
        : dataset_view{dset, std::forward<RandomEngine>(rng)},
          label_fn_{[this](const instance_type& instance)
                    {
                        return this->dset<binary_dataset>().label(instance);
                    }}
    {
        // nothing
    }

    binary_dataset_view(const binary_dataset_view& bdv, iterator begin,
                        iterator end)
        : dataset_view{bdv, begin, end}, label_fn_{bdv.label_fn_}
    {
        // nothing
    }

    template <class LabelFunction>
    binary_dataset_view(const multiclass_dataset_view& mdv, LabelFunction&& fn)
        : dataset_view{mdv}, label_fn_(std::forward<LabelFunction>(fn))
    {
        // nothing
    }

    template <class LabelFunction>
    binary_dataset_view(const multiclass_dataset_view& mdv,
                        std::vector<size_type>&& indices, LabelFunction&& fn)
        : dataset_view{mdv, std::move(indices)},
          label_fn_(std::forward<LabelFunction>(fn))
    {
        // nothing
    }

    bool label(const instance_type& instance) const
    {
        return label_fn_(instance);
    }

    size_type total_labels() const
    {
        return 2;
    }

  private:
    /// function to obtain the labels for instances
    std::function<bool(const instance_type&)> label_fn_;
};
}
}
#endif
