/**
 * @file multiclass_dataset.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_MULTICLASS_DATASET_H_
#define META_CLASSIFY_MULTICLASS_DATASET_H_

#include "meta/config.h"
#include "meta/learn/dataset.h"
#include "meta/util/invertible_map.h"

namespace meta
{
namespace classify
{

class multiclass_dataset : public learn::labeled_dataset<class_label>
{
  public:
    using class_label_map = util::invertible_map<label_type, label_id>;
    using class_label_iterator = class_label_map::iterator;

    /**
     * Creates an in-memory dataset from a forward_index. This loads the
     * **entire index** into memory, so you should only use this
     * constructor with small datasets.
     *
     * For large datasets (where large is defined as "larger than available
     * RAM", use one of the constructors that takes a range (or collection)
     * of document ids to load in to load in just a specific section of the
     * index.
     */
    multiclass_dataset(std::shared_ptr<index::forward_index> idx)
        : multiclass_dataset(
              idx, util::range(doc_id{0}, doc_id{idx->num_docs() - 1}))
    {
        // nothing
    }

    /**
     * Creates an in-memory dataset from a forward_index and a list of
     * document ids.
     */
    template <class DocIdContainer>
    multiclass_dataset(std::shared_ptr<index::forward_index> idx,
                       DocIdContainer&& dcont)
        : multiclass_dataset(idx, std::begin(dcont), std::end(dcont))
    {
        // nothing
    }

    /**
     * Creates an in-memory dataset from a forward_index and a range of
     * doc_ids, represented as iterators.
     */
    template <class ForwardIterator>
    multiclass_dataset(std::shared_ptr<index::forward_index> idx,
                       ForwardIterator begin, ForwardIterator end)
        : labeled_dataset{idx, begin, end,
                          [&](doc_id did) { return idx->label(did); }}
    {
        // build label_id_mapping
        for (const auto& lbl : idx->class_labels())
        {
            assert(label_id_mapping_.size()
                   < std::numeric_limits<uint32_t>::max());
            label_id_mapping_.insert(
                lbl, label_id(static_cast<uint32_t>(label_id_mapping_.size())));
        }
    }

    /**
     * Creates an in-memory listing of documents from an inverted_index
     * and a range of doc_ids, represented as iterators. Note that this
     * constructor will **not** load any feature_vectors, nor any
     * class_labels, as this is just a thin wrapper around a set of
     * document ids. This is mainly for use with the knn classifier.
     */
    template <class ForwardIterator>
    multiclass_dataset(std::shared_ptr<index::inverted_index> idx,
                       ForwardIterator begin, ForwardIterator end)
        : labeled_dataset{idx, begin, end}
    {
        // nothing
    }

    /**
     * Creates an in-memory listing of documents from an inverted_index.
     * Note that this constructor will **not** load any feature_vectors,
     * nor any class_labels, as this is just a thin wrapper around a set of
     * document ids. This is mainly for use with the knn classifier.
     */
    multiclass_dataset(std::shared_ptr<index::inverted_index> idx)
        : multiclass_dataset(idx,
                             util::range(0_did, doc_id(idx->num_docs() - 1)))
    {
        // nothing
    }

    /**
     * Creates an in-memory listing of documents from an inverted_index
     * and a container of doc_ids. Note that this constructor will **not**
     * load any feature_vectors, nor any class_labels, as this is just a
     * thin wrapper around a set of document ids. This is mainly for use
     * with the knn classifier.
     */
    template <class DocIdContainer>
    multiclass_dataset(std::shared_ptr<index::inverted_index> idx,
                       DocIdContainer&& cont)
        : multiclass_dataset(idx, std::begin(cont), std::end(cont))
    {
        // nothing
    }

    /**
     * Creates an in-memory dataset from a pair of iterators. The
     * dereferenced type must have a conversion operator to a
     * feature_vector and a conversion operator to a class_label.
     */
    template <class ForwardIterator>
    multiclass_dataset(ForwardIterator begin, ForwardIterator end)
        : labeled_dataset{begin, end}
    {
        // build label_id_mapping
        for (; begin != end; ++begin)
        {
            if (!label_id_mapping_.contains_key(*begin))
                label_id_mapping_.insert(*begin,
                                         label_id(label_id_mapping_.size()));
        }
    }

    /**
     * Creates an in-memory dataset from a pair of iterators, a function
     * to convert to a feature_vector and a function to obtain a label.
     */
    template <class ForwardIterator, class FeatureVectorFunction,
              class LabelFunction>
    multiclass_dataset(ForwardIterator begin, ForwardIterator end,
                       size_type total_features,
                       FeatureVectorFunction&& featurizer,
                       LabelFunction&& labeller)
        : labeled_dataset{begin, end, total_features,
                          std::forward<FeatureVectorFunction>(featurizer),
                          std::forward<LabelFunction>(labeller)}
    {
        // build label_id_mapping
        for (; begin != end; ++begin)
        {
            if (!label_id_mapping_.contains_key(labeller(*begin)))
                label_id_mapping_.insert(labeller(*begin),
                                         label_id(label_id_mapping_.size()));
        }
    }

    /**
     * @return the number of unique labels in the dataset
     */
    size_type total_labels() const
    {
        return label_id_mapping_.size();
    }

    /**
     * @return the label_id associated with this label
     */
    label_id label_id_for(const class_label& lbl) const
    {
        if (label_id_mapping_.empty())
            throw std::runtime_error{
                "no labels were loaded; did you mistakenly construct a dataset "
                "from an inverted_index instead of a forward_index?"};

        return label_id_mapping_.get_value(lbl);
    }

    /**
     * @return the class_label associated with this label_id
     */
    class_label label_for(label_id lid) const
    {
        if (label_id_mapping_.empty())
            throw std::runtime_error{
                "no labels were loaded; did you mistakenly construct a dataset "
                "from an inverted_index instead of a forward_index?"};

        return label_id_mapping_.get_key(lid);
    }

    class_label_iterator labels_begin() const
    {
        return label_id_mapping_.begin();
    }

    class_label_iterator labels_end() const
    {
        return label_id_mapping_.end();
    }

    void print_liblinear(std::ostream& os, const instance_type& instance) const
    {
        // SVM multiclass has label_ids starting at 1
        os << label_id_for(label(instance)) + 1;
        instance.print_liblinear(os);
    }

  private:
    /// the mapping from label <-> label_id
    class_label_map label_id_mapping_;
};
}
}
#endif
