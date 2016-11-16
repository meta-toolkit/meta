/**
 * @file dataset.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LEARN_TRANSFORM_H_
#define META_LEARN_TRANSFORM_H_

#include "meta/index/ranker/ranker.h"
#include "meta/index/score_data.h"
#include "meta/learn/dataset.h"

namespace meta
{
namespace learn
{

/**
 * Transformer for converting term frequency vectors into tf-idf weight
 * vectors. This transformation is performed with respect to a specific
 * index::inverted_index that defines the term statistics, and with respect
 * to an index::ranking_function that defines the "tf-idf" weight (via its
 * score_one() function).
 *
 * For example, one can construct a tfidf_transformer with an
 * inverted index and an okapi_bm25 ranker to get tf-idf vectors using
 * Okapi BM25's definitions of tf and idf.
 *
 * Some caveats to be aware of:
 *
 * 1. if your ranker uses extra information that isn't present in score_data
 *    (e.g. by using score_data.d_id and querying something), this will only
 *    work if your instance ids directly correspond to doc ids in the
 *    inverted index
 *
 * 2. tf-idf values are computed using statistics from the inverted_index.
 *    If this index contains your test set, the statistics are going to be
 *    computed including documents in your test set. If this is
 *    undesirable, create an inverted_index on just your training data and
 *    use that instead of one created on both the training and testing
 *    data.
 *
 * 3. This transformation only makes sense if your instances' weight
 *    vectors are actually term frequency vectors. If they aren't, the
 *    assumptions here that every entry in every weight vector can be
 *    safely converted to an integral value without rounding is violated.
 */
class tfidf_transformer
{
  public:
    /**
     * @param idx The index to use for term statistics
     * @param r The ranker to use for defining the weights
     */
    tfidf_transformer(index::inverted_index& idx, index::ranking_function& r)
        : idx_(idx),
          rnk_(r),
          sdata_(idx, idx.avg_doc_length(), idx.num_docs(),
                 idx.total_corpus_terms(), 1)
    {
        sdata_.query_term_weight = 1.0f;
    }

    /**
     * @param inst The instance to transform
     */
    void operator()(learn::instance& inst)
    {
        sdata_.d_id = doc_id{inst.id};
        sdata_.doc_size = static_cast<uint64_t>(std::accumulate(
            inst.weights.begin(), inst.weights.end(), 0.0,
            [](double accum, const std::pair<feature_id, double>& val) {
                return accum + val.second;
            }));
        sdata_.doc_unique_terms = inst.weights.size();
        for (auto& pr : inst.weights)
        {
            sdata_.t_id = term_id{pr.first};
            sdata_.doc_count = idx_.doc_freq(sdata_.t_id);
            sdata_.corpus_term_count = idx_.total_num_occurences(sdata_.t_id);
            sdata_.doc_term_count = static_cast<uint64_t>(pr.second);

            pr.second = rnk_.score_one(sdata_);
        }
    }

  private:
    index::inverted_index& idx_;
    index::ranking_function& rnk_;
    index::score_data sdata_;
};

/**
 * Transformer to normalize all unit vectors to unit length.
 */
class l2norm_transformer
{
  public:
    void operator()(learn::instance& inst) const
    {
        auto norm = std::sqrt(std::accumulate(
            inst.weights.begin(), inst.weights.end(), 0.0,
            [](double accum, const std::pair<feature_id, double>& val) {
                return accum + val.second * val.second;
            }));
        for (auto& pr : inst.weights)
            pr.second /= norm;
    }
};

/**
 * Transforms the feature vectors of a dataset **in place** using the given
 * transformation function. TransformFunction must have an operator() that
 * takes a learn::instance by mutable reference and changes its
 * feature values in-place. For example, a simple TransformFunction might
 * be one that normalizes all of the feature vectors to be unit length.
 *
 * @param dset The dataset to be transformed
 * @param trans The transformation function to be applied to all
 * feature_vectors in dset
 */
template <class TransformFunction>
void transform(dataset& dset, TransformFunction&& trans)
{
    for (auto& inst : dset)
        trans(inst);
}

/**
 * Transforms the feature vectors of a dataset **in place** to be tf-idf
 * features using the given index for term statistics and ranker for
 * tf-idf weight definitions.
 *
 * @param dset The dataset to be transformed
 * @param idx The inverted_index to use for term statistics like df
 * @param rnk The ranker to use to define tf-idf weights (via its
 * score_one())
 */
void tfidf_transform(dataset& dset, index::inverted_index& idx,
                     index::ranking_function& rnk)
{
    tfidf_transformer transformer{idx, rnk};
    transform(dset, transformer);
}

/**
 * Transforms the feature vectors of a dataset **in place** to be unit
 * length according to their L2 norm.
 *
 * @param dset The dataset to be transformed
 */
void l2norm_transform(dataset& dset)
{
    return transform(dset, l2norm_transformer{});
}
}
}
#endif
