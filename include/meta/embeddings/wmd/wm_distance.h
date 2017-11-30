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
/**
 * Struct representing one document in the wmd processing
 */
struct emb_document
{
    size_t n_terms;
    std::vector<size_t> ids;
    std::vector<double> weights;
};
/**
 * Class, providing methods to calculate distance between two documents
 * in a sense of word-embedding representation
 */
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

    /**
     * Calculates distance based on type of algorithm
     * @param algorithm_type type of the algorithm: "wcd", "rwmd" or "emd"
     * @param doc1
     * @param doc2
     * @return distance between two documents
     */
    double score(const std::string algorithm_type, const emb_document& doc1,
                 const emb_document& doc2);

    /**
     * Calculates original word mover's distance (based on Matt J. Kusner's
     * paper)
     * Uses Orif Pele Fast EMD algorithm
     * @param doc1
     * @param doc2
     * @return distance between two documents
     */
    double emd(const emb_document& doc1, const emb_document& doc2);
    /**
     * Calculates relaxed EM distance
     * @param doc1
     * @param doc2
     * @return distance between two documents
     */
    double emd_relaxed(const emb_document& doc1, const emb_document& doc2);
    /**
     * Calculates World Centroid distance
     * @param doc1
     * @param doc2
     * @return distance between two documents
     */
    double wcd(const emb_document& doc1, const emb_document& doc2);

    /**
     * L2 norm squared of the difference between two word embeddings
     * |a - b|2^2
     * @param a
     * @param b
     * @return distance between two word embeddings
     */
    static double l2diff_norm(const util::array_view<const double>& a,
                              const util::array_view<const double>& b);

    /**
     * Cosine measure between two word embeddings
     * Since we want minimum between two similar terms it calculates (1 - cos)/2
     * @param a
     * @param b
     * @return distance between two word embeddings
     */
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

    /**
     * Returns distance between two terms using cache
     * @param first_word_id first term id
     * @param second_word_id second term id
     * @return distance between two terms
     */
    double f_c_distance(const size_t first_word_id,
                        const size_t second_word_id);
};
}
}

#endif // META_EMD_H
