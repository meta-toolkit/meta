/**
 * @file wmd_base.h
 * @author lolik111
 */

#ifndef META_WMD_BASE_H
#define META_WMD_BASE_H

#include "meta/embeddings/wmd/wm_distance.h"
#include "meta/embeddings/word_embeddings.h"
#include "meta/index/ranker/ranker.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/util/array_view.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace index
{

/**
 * Implements word mover's distance model.
 *
 * @see http://mkusner.github.io/publications/WMD.pdf
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "wmd"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * mode                # current mode: can be "emd", "wcd", "rwmd", or
 * "prefetch-prune"
 * distance-func       # type of the distance function: "l2diff" or "cosine"
 * num-threads         # number of threads used in the algorithm
 * cache-per-thread    # size of cache per each thread
 * ~~~
 */
class wmd_base : public ranker
{
  public:
    /// Identifier for this ranker.
    const static util::string_view id;

    const static std::string default_mode;

    const static std::string default_distance_func;

    const static constexpr size_t default_cache_size = 1000000;

    wmd_base(std::shared_ptr<forward_index> fwd,
             std::shared_ptr<embeddings::word_embeddings> embeddings,
             size_t nthreads, size_t cache_size, std::string mode,
             std::string distance_func);

    wmd_base(std::istream& in);

    void save(std::ostream& out) const override;

    std::vector<search_result>
    rank(ranker_context& ctx, uint64_t num_results,
         const filter_function_type& filter) override;

  private:
    std::shared_ptr<forward_index> fwd_;
    std::shared_ptr<embeddings::word_embeddings> embeddings_;
    const size_t nthreads_;
    const size_t cache_size_;
    std::shared_ptr<caching::dblru_shard_cache<std::pair<uint64_t, uint64_t>,
                                               double>>
        cache_;
    const std::string mode_;
    const std::string distance_func_;
    /**
     * Creates document, omitting terms not presenting in the embeddings
     * @param tf vector of term frequences
     * @return Struct representing one document in the wmd processing
     */
    embeddings::emb_document
    create_document(std::vector<std::pair<term_id, double>> tf);

    /**
     * Calculates wmd based on the instance of the emd class and mode paralelly
     * @param emd
     * @param mode
     * @param filter
     * @param doc_to_compare
     * @param docs documents
     * @return vector of search results
     */
    std::vector<search_result> process(embeddings::wm_distance emd,
                                       const std::string mode,
                                       const filter_function_type& filter,
                                       embeddings::emb_document doc_to_compare,
                                       std::vector<doc_id> docs);
};

/**
 * Specialization of the factory method used to create wmd
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<wmd_base>(const cpptoml::table& global,
                                              const cpptoml::table& local);
}
}
#endif
