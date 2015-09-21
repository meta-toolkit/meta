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
#include "learn/dataset.h"
#include "util/comparable.h"
#include "util/functional.h"

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
        indices_.reserve(std::distance(first, last));
        for (; first != last; ++first)
            indices_.emplace_back(first.index());
    }

    void shuffle()
    {
        // THERE IS A REASON we don't use std::shuffle here: we want
        // reproducibility between compilers, who don't seem to agree on
        // the number of times to call rng_ in the shuffle process.
        //
        // Furthermore, it seems that we can't rely on a canonical number
        // of rng_ calls in std::uniform_int_distribution, either, so
        // that's out too.
        //
        // We instead use functional::bounded_rand(), since we know that
        // the range of the RNG is definitely going to be larger than the
        // upper bounds we request here

        for (std::size_t i = 0; i < indices_.size(); ++i)
        {
            using std::swap;
            swap(indices_[indices_.size() - 1 - i],
                 indices_[functional::bounded_rand(rng_,
                                                   indices_.size() - i)]);
        }
    }

    void rotate(size_type block_size)
    {
        std::rotate(indices_.begin(), indices_.begin() + block_size,
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

  private:
    const dataset* dset_;
    std::vector<size_type> indices_;

    // type erase any random number generator in a way that still makes STL
    // algorithms happy
    class any_rng
    {
      public:
        using result_type = std::uint64_t;
        static constexpr result_type min()
        {
            return 0;
        }

        static constexpr result_type max()
        {
            return std::numeric_limits<result_type>::max();
        }

        template <class RandomEngine>
        using random_engine
            = std::independent_bits_engine<RandomEngine, 64, result_type>;

        template <class RandomEngine,
                  class = typename std::
                      enable_if<!std::is_same<
                                    typename std::decay<RandomEngine>::type,
                                    any_rng>::value>::type>
        any_rng(RandomEngine&& rng)
            : wrapped_(random_engine<typename std::decay<RandomEngine>::type>(
                  std::forward<RandomEngine>(rng)))
        {
            // nothing
        }

        result_type operator()() const
        {
            return wrapped_();
        }

      private:
        std::function<result_type()> wrapped_;
    } rng_;
};
}
}
#endif
