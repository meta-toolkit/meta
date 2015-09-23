/**
 * @file dataset.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LEARN_DATASET_H_
#define META_LEARN_DATASET_H_

#include <memory>
#include "corpus/metadata.h"
#include "index/forward_index.h"
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "util/progress.h"
#include "util/range.h"
#include "util/sparse_vector.h"
#include "util/identifiers.h"

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

/**
 * Represents an in-memory view of a set of documents for running learning
 * algorithms over.
 */
class dataset
{
  public:
    using instance_type = instance;
    using const_iterator = std::vector<instance_type>::const_iterator;
    using iterator = const_iterator;
    using size_type = std::vector<instance_type>::size_type;

    /**
     * Creates an in-memory dataset from a forward_index and a range of
     * doc_ids, represented as iterators.
     */
    template <class ForwardIterator>
    dataset(std::shared_ptr<index::forward_index> idx, ForwardIterator begin,
            ForwardIterator end)
        : total_features_{idx->unique_terms()}
    {
        uint64_t size = std::distance(begin, end);
        instances_.reserve(size);

        printing::progress progress{" > Loading instances into memory: ", size};
        for (auto doc = 0_inst_id; begin != end; ++begin, ++doc)
        {
            progress(doc);
            auto stream = idx->stream_for(*begin);
            instances_.emplace_back(doc, stream->begin(), stream->end());
        }
    }

    /**
     * Creates an in-memory listing of documents from an inverted_index
     * and a range of doc_ids, represented as iterators. Note that this
     * constructor will **not** load any feature_vectors, as doing so from
     * an inverted index isn't possible. This ctor is mainly for use with
     * the knn classifier. The id field of the instance_types stored within
     * the dataset is a document_id.
     */
    template <class ForwardIterator>
    dataset(std::shared_ptr<index::inverted_index> idx, ForwardIterator begin,
            ForwardIterator end)
        : total_features_{idx->unique_terms()}
    {
        uint64_t size = std::distance(begin, end);
        instances_.reserve(size);

        printing::progress progress{" > Loading instances into memory: ", size};
        for (uint64_t pos = 0; begin != end; ++begin, ++pos)
        {
            progress(pos);
            instances_.emplace_back(instance_id(*begin));
        }
    }

    /**
     * Creates an in-memory dataset from a pair of iterators. The
     * dereferenced type must have a conversion operator to a
     * feature_vector.
     */
    template <class ForwardIterator>
    dataset(ForwardIterator begin, ForwardIterator end,
            size_type total_features)
        : total_features_{total_features}
    {
        instances_.reserve(std::distance(begin, end));
        auto id = 0_inst_id;
        for (; begin != end; ++begin, ++id)
            instances_.emplace_back(id, *begin);
    }

    /**
     * @return an iterator to the first instance
     */
    iterator begin() const
    {
        return instances_.begin();
    }

    /**
     * @return an iterator to one past the end of the dataset
     */
    iterator end() const
    {
        return instances_.end();
    }

    /**
     * @return the size of the dataset
     */
    size_type size() const
    {
        return instances_.size();
    }

    /**
     * @return the number of features in the dataset
     */
    size_type total_features() const
    {
        return total_features_;
    }

    /**
     * @param index The index of the item you want in the dataset. Note
     * that the index is **not** a doc_id!
     *
     * Index == 0 does not imply doc_id == 0.
     *
     * @return the instance at that index in the dataset
     */
    const instance_type& operator()(size_type index) const
    {
        return instances_.at(index);
    }

  private:
    /// the instances themselves
    std::vector<instance_type> instances_;
    /// the total number of unique features in the dataset
    size_type total_features_;
};

template <class LabelType>
class labeled_dataset : public dataset
{
  public:
    using label_type = LabelType;

    /**
     * Creates an in-memory dataset from a forward_index and a range of
     * doc_ids, represented as iterators.
     */
    template <class ForwardIterator, class LabelFunction>
    labeled_dataset(std::shared_ptr<index::forward_index> idx,
                    ForwardIterator begin, ForwardIterator end,
                    LabelFunction&& labeller)
        : dataset{idx, begin, end}
    {
        labels_.reserve(size());
        std::transform(begin, end, std::back_inserter(labels_), labeller);
    }

    /**
     * Creates an in-memory dataset from an inverted_index and a range fo
     * doc_ids, represented as iterators. Note that this does **not**
     * actually load any feature_vectors or LabelTypes, as this is just a
     * thin wrapper around a set of document ids. This is mainly for use
     * with the knn classifier.
     */
    template <class ForwardIterator>
    labeled_dataset(std::shared_ptr<index::inverted_index> idx,
                    ForwardIterator begin, ForwardIterator end)
        : dataset{idx, begin, end}
    {
        // nothing
    }

    /**
     * Creates an in-memory dataset from a pair of iterators. The
     * dereferenced type must have a conversion operator to a
     * feature_vector and a conversion operator to a LabelType.
     */
    template <class ForwardIterator>
    labeled_dataset(ForwardIterator begin, ForwardIterator end,
                    size_type total_features)
        : dataset{begin, end, total_features}, labels_{begin, end}
    {
        // nothing
    }

    /**
     * @return the label for an instance
     */
    label_type label(const instance_type& inst) const
    {
        if (labels_.empty())
            throw std::runtime_error{
                "no labels were loaded; did you "
                "mistakenly construct a dataset from an "
                "inverted_index instead of a forward_index?"};
        return labels_.at(inst.id);
    }

  private:
    /// the (dense) mapping from instance_id -> class_label
    std::vector<label_type> labels_;
};
}
}
#endif
