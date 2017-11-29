/**
 * @file wmd_base.h
 * @author lolik111
 */

#ifndef META_WMD_BASE_H
#define META_WMD_BASE_H

#include "meta/embeddings/word_embeddings.h"
#include "meta/index/ranker/emd.h"
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
 * mode             # current mode: can be 'emd', 'wcd-emd', or 'rwmd'
 * num-threads          # number of threads used in the algorithm
 * cache-per-thread # size of cache per each thread
 * ~~~
 */
class wmd_base : public ranker
{
  public:
    /// Identifier for this ranker.
    const static util::string_view id;

    const static std::string default_mode;

    const static constexpr size_t default_cache_size = 1000000;

    wmd_base(std::shared_ptr<forward_index> fwd,
             std::shared_ptr<embeddings::word_embeddings> embeddings,
             size_t nthreads, size_t cache_size);

    wmd_base(std::istream& in);

    void save(std::ostream& out) const override;

    std::vector<search_result>
    rank(ranker_context& ctx, uint64_t num_results,
         const filter_function_type& filter) override;

  private:
    std::shared_ptr<forward_index> fwd_;
    std::shared_ptr<caching::dblru_shard_cache<std::pair<uint64_t, uint64_t>,
                                               double>>
        cache_;
    std::shared_ptr<embeddings::word_embeddings> embeddings_;
    const size_t nthreads_;
    const size_t cache_size_;

    meta::index::Document
    create_document(std::vector<std::pair<term_id, double>> tf,
                    ranker_context& ctx);
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
