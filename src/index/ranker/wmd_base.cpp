/**
 * @file wmd_base.cpp
 * @author lolik111
 */

#include "meta/index/ranker/wmd_base.h"
#include "meta/index/forward_index.h"
#include "meta/index/postings_data.h"
#include "meta/index/score_data.h"
#include "meta/util/fixed_heap.h"

namespace meta
{
namespace index
{
const util::string_view wmd_base::id = "wmd-base";

const std::string wmd_base::default_mode = "rwmd";

const std::string wmd_base::default_distance_func = "cosine";

const constexpr size_t wmd_base::default_cache_size;

wmd_base::wmd_base(std::shared_ptr<forward_index> fwd,
                   std::shared_ptr<embeddings::word_embeddings> embeddings,
                   size_t nthreads, size_t cache_size, std::string mode,
                   std::string distance_func)
    : fwd_(fwd),
      embeddings_(embeddings),
      nthreads_(nthreads),
      cache_size_(cache_size),
      cache_{std::make_shared<caching::dblru_shard_cache<std::pair<uint64_t,
                                                                   uint64_t>,
                                                         double>>(nthreads,
                                                                  cache_size)},
      mode_(mode),
      distance_func_(distance_func)
{
}

void wmd_base::save(std::ostream& out) const
{
    io::packed::write(out, id);
    io::packed::write(out, nthreads_);
    io::packed::write(out, cache_size_);
    io::packed::write(out, mode_);
    io::packed::write(out, distance_func_);
    io::packed::write(out, fwd_->index_name());
}

wmd_base::wmd_base(std::istream& in)
    : nthreads_{io::packed::read<uint64_t>(in)},
      cache_size_{io::packed::read<uint64_t>(in)},
      cache_{std::make_shared<caching::dblru_shard_cache<std::pair<uint64_t,
                                                                   uint64_t>,
                                                         double>>(nthreads_,
                                                                  cache_size_)},
      mode_{io::packed::read<std::string>(in)},
      distance_func_{io::packed::read<std::string>(in)}
{
    auto path = io::packed::read<std::string>(in);
    auto cfg = cpptoml::parse_file(path + "/config.toml");
    fwd_ = make_index<forward_index>(*cfg);

    embeddings_ = std::make_shared<embeddings::word_embeddings>(
        embeddings::load_embeddings(*cfg));
}

std::vector<search_result> wmd_base::rank(ranker_context& ctx,
                                          uint64_t num_results,
                                          const filter_function_type& filter)
{
    auto results = util::make_fixed_heap<search_result>(
        num_results, [](const search_result& a, const search_result& b) {
            return a.score < b.score;
        });

    em_distance::metric_type distance;
    if (distance_func_ == "cosine")
    {
        distance = em_distance::cosine;
    }
    else if (distance_func_ == "l2diff")
    {
        distance = em_distance::l2diff_norm;
    }
    else
    {
        distance = em_distance::cosine;
    }

    parallel::thread_pool pool(nthreads_);
    std::vector<doc_id> docs = fwd_->docs();

    if (mode_ != "prefetch-prune")
    {
        meta::index::em_distance emd(cache_, embeddings_, mode_, distance);
        auto scores = process(emd, filter, ctx, fwd_->docs());
        for (auto score : scores)
        {
            results.emplace(score);
        }
    }
    else
    {
        index::em_distance wcd(cache_, embeddings_, "wcd", em_distance::l2diff_norm);
        index::em_distance emd(cache_, embeddings_, "emd", distance);
        index::em_distance rwmd(cache_, embeddings_, "rwmd", distance);

        // wcd phase
        auto scores = process(wcd, filter, ctx, fwd_->docs());
        std::sort(scores.begin(), scores.end(),
                  [&](const search_result a, const search_result b) {
                      bool ans;
                      ans = a.score < b.score;
                      return ans;
                  });

        auto emd_heap = util::make_fixed_heap<search_result>(
            num_results, [](const search_result& a, const search_result& b) {
                return a.score < b.score;
            });
        std::vector<doc_id> k_docs;
        for (size_t i = 0; i < num_results; i++)
        {
            k_docs.push_back(scores[i].d_id);
        }
        scores.erase(scores.begin(), scores.begin() + num_results);
        // emd after wcd
        auto k_emd = process(emd, filter, ctx, k_docs);
        for(auto sr : k_emd)
        {
            results.emplace(sr);
        }

        // worst result
        auto last = (--results.end())->score;

        const size_t magic_constant = std::max(fwd_->docs().size() / 8,
                                               num_results * 8);
        std::vector<doc_id> rwmd_docs(magic_constant);
        auto start = scores.begin();
        std::generate(rwmd_docs.begin(), rwmd_docs.end(), [&](){
            return (*start++).d_id;
        });
        // rwmd phase
        auto rwmd_results = process(rwmd, filter, ctx, rwmd_docs);

        std::vector<doc_id> pretend_docs;

        for(auto sr : rwmd_results)
        {
            if (sr.score < last)
            {
                pretend_docs.emplace_back(sr.d_id);
            }
        }

        if (!pretend_docs.empty())
        {   // emd phase
            auto pretend_results = process(emd, filter, ctx, pretend_docs);
            for (auto sr : pretend_results)
            {
                results.emplace(sr);
            }
        }

    }

    return results.extract_top();
}

std::vector<search_result> wmd_base::process(em_distance emd,
                                             const filter_function_type& filter,
                                             ranker_context& ctx,
                                             std::vector<doc_id> docs)
{
    parallel::thread_pool pool(nthreads_);

    auto scores = parallel::for_each_block(
        docs.begin(), docs.end(), pool, [&](std::vector<doc_id>::iterator start,
                                            std::vector<doc_id>::iterator end) {
            std::vector<search_result> block_scores;
            for (auto it = start; it != end; ++it)
            {
                if (!filter(*it))
                    continue;
                auto tf = fwd_->search_primary(*it)->counts();
                auto doc1 = create_document(tf);

                std::vector<std::pair<term_id, double>> tf_pc;
                tf_pc.reserve(ctx.postings.size());
                for (auto one : ctx.postings)
                {
                    tf_pc.push_back({one.t_id, one.query_term_weight});
                }

                auto doc2 = create_document(tf_pc);
                if(doc1.n_terms == 0 || doc2.n_terms == 0){
                    continue;
                }
                auto score = static_cast<float>(emd.score(doc1, doc2));
                block_scores.emplace_back(*it, score);
            }
            return block_scores;
        });
    std::vector<search_result> results;
    results.reserve(fwd_->docs().size());
    for (auto& vec : scores)
    {
        for (auto sr : vec.get())
        {
            results.emplace_back(sr);
        }
    }
    return results;
}

meta::index::Document
wmd_base::create_document(std::vector<std::pair<term_id, double>> tf)
{
    size_t unique_terms_count = tf.size();
    size_t all_terms_count = 0;

    meta::index::Document document;
    document.ids = std::vector<size_t>();
    document.ids.reserve(unique_terms_count);
    document.weights = std::vector<double>();
    document.weights.reserve(unique_terms_count);

    for (auto term_data : tf)
    {
        std::string term = fwd_->term_text(term_data.first);
        auto vec_id = this->embeddings_->tid(term);

        if (vec_id >= 0)
        {
            all_terms_count += term_data.second;
            document.weights.emplace_back(term_data.second);
            document.ids.emplace_back(vec_id);
        }
        else
        {
            unique_terms_count--;
        }
    }

    using namespace meta::math::operators;

    document.weights = document.weights / all_terms_count;
    document.n_terms = unique_terms_count;

    return document;
}

template <>
std::unique_ptr<ranker> make_ranker<wmd_base>(const cpptoml::table& global,
                                              const cpptoml::table& local)
{
    if (global.begin() == global.end())
        throw ranker_exception{"empty global configuration provided to "
                               "construction of wmd_base ranker"};
    auto f_idx = make_index<forward_index>(global);

    auto embeddings = global.get_table("embeddings");
    if (!embeddings)
        throw std::runtime_error{"\"embeddings\" group needed in config file!"};

    auto glove = embeddings::load_embeddings(*embeddings);

    auto cache_size = local.get_as<size_t>("cache-per-thread")
                          .value_or(wmd_base::default_cache_size);
    size_t nthreads = local.get_as<size_t>("num-threads")
                          .value_or(std::thread::hardware_concurrency());

    auto mode
        = local.get_as<std::string>("mode").value_or(wmd_base::default_mode);

    auto distance_func = local.get_as<std::string>("distance-func")
                             .value_or(wmd_base::default_distance_func);

    return make_unique<wmd_base>(
        f_idx, std::make_shared<embeddings::word_embeddings>(glove), nthreads,
        cache_size, mode, distance_func);
}
}
}
