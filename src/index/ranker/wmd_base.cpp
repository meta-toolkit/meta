//
// Created by lolik111 on 17.11.17.
//

#include <meta/util/time.h>
#include "meta/index/ranker/wmd_base.h"
#include "meta/index/postings_data.h"
#include "meta/util/fixed_heap.h"
#include "meta/index/score_data.h"
#include "meta/logging/logger.h"
#include "meta/index/forward_index.h"


namespace meta
{
namespace index
{


const util::string_view wmd_base::id = "wmd-base";

const std::string wmd_base::default_mode = "rwmd";

const constexpr size_t wmd_base::default_cache_size;

wmd_base::wmd_base(std::shared_ptr<forward_index> fwd,
                   std::shared_ptr<embeddings::word_embeddings> embeddings,
                   size_t cache_size, size_t nthreads)
        : fwd_{std::move(fwd)},
          embeddings_{embeddings},
          nthreads_{nthreads},
          cache_size_{cache_size},
          cache_{std::make_shared<caching::dblru_shard_cache<std::pair
                <uint64_t, uint64_t>, double> > (nthreads, cache_size)}
{

}

void wmd_base::save(std::ostream &out) const
{
    io::packed::write(out, id);
    io::packed::write(out, nthreads_);
    io::packed::write(out, cache_size_);
    io::packed::write(out, fwd_->index_name());

}

wmd_base::wmd_base(std::istream &in) :
          nthreads_{io::packed::read<uint64_t>(in)},
          cache_size_{io::packed::read<uint64_t>(in)},
          cache_{std::make_shared<caching::dblru_shard_cache<std::pair
                  <uint64_t, uint64_t>, double> >(nthreads_,
                                                  cache_size_)}
{
    auto path = io::packed::read<std::string>(in);
    auto cfg = cpptoml::parse_file(path + "/config.toml");
    fwd_ = make_index<forward_index>(*cfg);

    embeddings_ = std::make_shared<embeddings::word_embeddings>
            (embeddings::load_embeddings(*cfg));
}


std::vector<search_result>
wmd_base::rank(ranker_context &ctx, uint64_t num_results,
               const filter_function_type &filter)
{
    auto results = util::make_fixed_heap<search_result>(
            num_results,
            [](const search_result &a, const search_result &b){
                return a.score < b.score;
            });

    meta::index::em_distance emd(cache_, index::em_distance::l2diff_norm,
                                 embeddings_->vector_size());

    for (auto doc : fwd_->docs()) {
        if (!filter(doc)) continue;

        std::vector<std::pair<term_id, double>> tf = fwd_->search_primary(
                doc)->counts();

        auto doc1 = create_document(tf, ctx);

        std::vector<std::pair<term_id, double>> tf_pc;
        std::vector<detail::postings_context> pc = ctx.postings;
        for (auto one: pc) {
            tf_pc.push_back(std::pair<term_id, double>(one.t_id,
                                                       one.query_term_weight));
        }

        auto doc2 = create_document(tf_pc, ctx);

//        double score1 = emd.emd_relaxed(doc1, doc2);
//        double score2 = emd.emd_relaxed(doc2, doc1);
//        results.emplace(search_result(doc, (float) std::max(score1, score2)));
        auto time
                = common::time([&]() {
                    results.emplace(search_result{doc, static_cast<float>(emd
                            .forward_emd(doc1, doc2))});
                });
        int p = 3;
    }

    return results.extract_top();

}

meta::index::Document
wmd_base::create_document(std::vector<std::pair<term_id, double>> tf,
                          ranker_context &ctx)
{
    size_t unique_terms_count = tf.size();
    size_t all_terms_count = 0;

    meta::index::Document document;
    document.vectors = std::vector<std::vector<double>>();
    document.vectors.reserve(unique_terms_count);
    document.ids = std::vector<size_t>();
    document.ids.reserve(unique_terms_count);
    document.weights = std::vector<double>();
    document.weights.reserve(unique_terms_count);

    for (auto term_data : tf)
    {
        std::string term = fwd_->term_text(term_data.first);

        auto vec_id = this->embeddings_->tid(term);

        if (vec_id >= 0) {
            all_terms_count += term_data.second;
            auto embedding = this->embeddings_->at(term);
            document.vectors.emplace_back(std::vector<double>(
                    embedding.v.begin(), embedding.v.end()));
            document.weights.emplace_back(term_data.second);
            document.ids.emplace_back(vec_id);

        } else {
            unique_terms_count--;
        }
    }

    using namespace meta::math::operators;

    document.weights = document.weights / unique_terms_count;
    document.n_terms = unique_terms_count;

    return document;
}


template<>
std::unique_ptr<ranker>
make_ranker<wmd_base>(const cpptoml::table &global,
                      const cpptoml::table &local)
{
    if (global.begin() == global.end())
        throw ranker_exception{"empty global configuration provided to "
                                       "construction of wmd_base ranker"};
    auto embeddings = global.get_table("embeddings");
    if (!embeddings)
        throw std::runtime_error{
                "\"embeddings\" group needed in config file!"};
    auto glove = embeddings::load_embeddings(*embeddings);

    auto mode = local.get_as<std::string>("mode").value_or
            (wmd_base::default_mode);
    auto cache_size = local.get_as<size_t>("cache-per-thread").value_or
            (wmd_base::default_cache_size);
    size_t nthreads = local.get_as<size_t>("num-threads").value_or
            (std::thread::hardware_concurrency());

    auto f_idx = make_index<forward_index>(global);

    return make_unique<wmd_base>(f_idx,
                                 std::make_shared<embeddings::word_embeddings>(
                                         glove), cache_size, nthreads);
}
}
}

