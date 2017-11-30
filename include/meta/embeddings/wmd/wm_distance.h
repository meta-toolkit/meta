/**
 * @file wm_distance.h
 * @author lolik111
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_EMD_H
#define META_EMD_H

#include <functional>
#include <vector>

#include "meta/caching/all.h"
#include "meta/embeddings/word_embeddings.h"
#include "meta/math/vector.h"

namespace meta
{

namespace embeddings
{

struct emb_document
{
    size_t n_terms;
    std::vector<size_t> ids;
    std::vector<double> weights;
};

class wm_distance
{
  public:
    using metric_type
        = std::function<double(const util::array_view<const double>&,
                               const util::array_view<const double>&)>;

    wm_distance(
        std::
            shared_ptr<caching::dblru_shard_cache<std::pair<uint64_t, uint64_t>,
                                                  double>>
                cache_,
        std::shared_ptr<embeddings::word_embeddings> embeddings,
        metric_type metric, size_t nthreads = 1);

    double score(const std::string algorithm_type, const emb_document& doc1,
                 const emb_document& doc2);

    double emd(const emb_document& doc1, const emb_document& doc2);

    double emd_relaxed(const emb_document& doc1, const emb_document& doc2);

    double wcd(const emb_document& doc1, const emb_document& doc2);

    static double l2diff_norm(const util::array_view<const double>& a,
                              const util::array_view<const double>& b);

    static double cosine(const util::array_view<const double>& a,
                         const util::array_view<const double>& b);

  private:
    const size_t nthreads_;
    std::shared_ptr<caching::dblru_shard_cache<std::pair<uint64_t, uint64_t>,
                                               double>>
        cache_;
    std::shared_ptr<embeddings::word_embeddings> embeddings_;
    const size_t dimension_;
    const metric_type dist;

    std::unordered_map<std::string, std::function<double(const emb_document&,
                                                         const emb_document&)>>
        methods_;

    double f_c_distance(const emb_document& doc1, const emb_document& doc2,
                        size_t first, size_t second);
};
}
}

#endif // META_EMD_H
