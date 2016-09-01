/**
 * @file dataset_view.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LEARN_DATASET_VIEW_H_
#define META_LEARN_DATASET_VIEW_H_

#include <iostream>
#include <iterator>

#include "meta/config.h"
#include "meta/learn/dataset.h"
#include "meta/util/comparable.h"
#include "meta/util/random.h"

namespace meta
{
namespace learn
{

/**
 * A non-owning view of a dataset. This is typically what the learning
 * algorithms will see: their own view of a dataset, which they can
 * shuffle/permute as they see fit. Derived classes of dataset_view will
 * typically also define the label value for each element in the dataset
 * that returns something appropriate for that kind of algorithm: +1 or -1
 * for binary classification, a class_label for multiclass, a double for
 * regression, etc.
 */
class dataset_view
{
  public:
    using instance_type = dataset::instance_type;
    using size_type = dataset::size_type;

    class iterator;

    dataset_view(const dataset& dset)
        : dataset_view{dset, std::mt19937_64{std::random_device{}()}}
    {
        // nothing
    }

    template <class RandomEngine>
    dataset_view(const dataset& dset, RandomEngine&& rng)
        : dset_{&dset},
          indices_(dset.size()),
          rng_(std::forward<RandomEngine>(rng))
    {
        std::iota(indices_.begin(), indices_.end(), 0);
    }

    // subset constructor
    dataset_view(const dataset_view& dv, iterator first, iterator last)
        : dset_{dv.dset_}, rng_{dv.rng_}
    {
        assert(first <= last);
        indices_.reserve(static_cast<std::size_t>(std::distance(first, last)));
        for (; first != last; ++first)
            indices_.emplace_back(first.index());
    }

    void add_by_index(size_type idx)
    {
        indices_.push_back(idx);
    }

    void shuffle()
    {
        // use meta::random::shuffle for reproducibility between compilers
        random::shuffle(indices_.begin(), indices_.end(), rng_);
    }

    void rotate(size_type block_size)
    {
        using diff_type = decltype(indices_.begin())::difference_type;
        std::rotate(indices_.begin(),
                    indices_.begin() + static_cast<diff_type>(block_size),
                    indices_.end());
    }

    class iterator : public std::iterator<std::random_access_iterator_tag,
                                          const instance_type>,
                     public util::comparable<iterator>
    {
      public:
        using difference_type = iterator::difference_type;

        iterator(const dataset* dset, std::vector<size_type>::const_iterator it)
            : dset_{dset}, it_{it}
        {
            // nothing
        }

        const instance_type& operator*() const
        {
            return (*dset_)(*it_);
        }

        const instance_type* operator->() const
        {
            return &(**this);
        }

        iterator& operator++()
        {
            ++it_;
            return *this;
        }

        iterator operator++(int)
        {
            auto ret = *this;
            ++(*this);
            return ret;
        }

        iterator& operator+=(difference_type n)
        {
            it_ += n;
            return *this;
        }

        iterator& operator-=(difference_type n)
        {
            it_ -= n;
            return *this;
        }

        friend iterator operator+(iterator it, difference_type n)
        {
            return it += n;
        }

        friend iterator operator+(difference_type n, iterator it)
        {
            return it += n;
        }

        friend iterator operator-(iterator it, difference_type n)
        {
            return it -= n;
        }

        friend iterator operator-(difference_type n, iterator it)
        {
            return it -= n;
        }

        friend difference_type operator-(iterator first, iterator last)
        {
            return first.it_ - last.it_;
        }

        const instance_type& operator[](difference_type n) const
        {
            return *(*this + n);
        }

        bool operator<(const iterator& it) const
        {
            return it_ < it.it_;
        }

        size_type index() const
        {
            return *it_;
        }

      private:
        const dataset* dset_;
        std::vector<size_type>::const_iterator it_;
    };
    using const_iterator = iterator;

    iterator begin() const
    {
        return {dset_, indices_.begin()};
    }

    iterator end() const
    {
        return {dset_, indices_.end()};
    }

    size_type size() const
    {
        return indices_.size();
    }

    size_type total_features() const
    {
        return dset_->total_features();
    }

  protected:
    // subset constructor v1
    dataset_view(const dataset_view& dv, std::vector<size_type>&& indices)
        : dset_{dv.dset_}, indices_{std::move(indices)}, rng_{dv.rng_}
    {
        // nothing
    }

    template <class DerivedDataset>
    const DerivedDataset& dset() const
    {
        return static_cast<const DerivedDataset&>(*dset_);
    }

    const std::vector<size_type> indices() const
    {
        return indices_;
    }

  private:
    const dataset* dset_;
    std::vector<size_type> indices_;

    // type erase any random number generator in a way that still makes STL
    // algorithms happy
    random::any_rng rng_;
};
}
}
#endif
