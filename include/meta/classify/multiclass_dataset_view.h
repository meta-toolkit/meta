/**
 * @file multiclass_dataset_view.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_MULTICLASS_DATASET_VIEW_H_
#define META_CLASSIFY_MULTICLASS_DATASET_VIEW_H_

#include "meta/classify/multiclass_dataset.h"
#include "meta/config.h"
#include "meta/learn/dataset_view.h"
#include "meta/logging/logger.h"

namespace meta
{
namespace classify
{

/**
 * A non-owning view of a dataset with categorical class labels. This is
 * designed to be used with mutli-class classifiers.
 */
class multiclass_dataset_view : public learn::dataset_view
{
  public:
    multiclass_dataset_view(const multiclass_dataset& dset) : dataset_view{dset}
    {
        // nothing
    }

    template <class RandomEngine>
    multiclass_dataset_view(const multiclass_dataset& dset, RandomEngine&& rng)
        : dataset_view{dset, std::forward<RandomEngine>(rng)}
    {
        // nothing
    }

    multiclass_dataset_view(const multiclass_dataset_view& mdv, iterator begin,
                            iterator end)
        : dataset_view{mdv, begin, end}
    {
        // nothing
    }

    multiclass_dataset_view(const multiclass_dataset_view& mdv,
                            std::vector<size_type>&& indices)
        : dataset_view{mdv, std::move(indices)}
    {
        // nothing
    }

    multiclass_dataset_view create_even_split() const
    {
        LOG(info) << "Creating an even split of class labels" << ENDLG;

        using indices_type = std::vector<size_type>;
        using diff_type = indices_type::iterator::difference_type;

        // partition by class label
        std::unordered_map<class_label, indices_type> partitioned;
        using value_type = decltype(partitioned)::value_type;
        for (auto it = begin(), end = this->end(); it != end; ++it)
            partitioned[label(*it)].push_back(it.index());

        // find the class with the least representation
        auto it
            = std::min_element(partitioned.begin(), partitioned.end(),
                               [](const value_type& a, const value_type& b) {
                                   return a.second.size() < b.second.size();
                               });

        // create the set of dataset indices that contains an equal number of
        // instances for each class label
        indices_type indices;
        indices.reserve(it->second.size() * partitioned.size());
        for (const auto& bucket : partitioned)
        {
            indices.insert(indices.end(), bucket.second.begin(),
                           bucket.second.begin()
                               + static_cast<diff_type>(it->second.size()));
        }

        LOG(info) << "Each of the " << partitioned.size() << " classes has "
                  << it->second.size()
                  << " elements for a baseline accuracy of "
                  << 1.0 / partitioned.size() << ENDLG;

        return {*this, std::move(indices)};
    }

    size_type total_labels() const
    {
        return dset<multiclass_dataset>().total_labels();
    }

    class_label label(const instance_type& instance) const
    {
        return dset<multiclass_dataset>().label(instance);
    }

    multiclass_dataset::class_label_iterator labels_begin() const
    {
        return dset<multiclass_dataset>().labels_begin();
    }

    multiclass_dataset::class_label_iterator labels_end() const
    {
        return dset<multiclass_dataset>().labels_end();
    }

    void print_liblinear(std::ostream& os, const instance_type& instance) const
    {
        return dset<multiclass_dataset>().print_liblinear(os, instance);
    }
};
}
}
#endif
