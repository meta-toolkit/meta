/**
 * @file rocchio.cpp
 * @author Chase Geigle
 */

#include "cpptoml.h"

#include "meta/hashing/probe_map.h"
#include "meta/index/forward_index.h"
#include "meta/index/ranker/okapi_bm25.h"
#include "meta/index/ranker/rocchio.h"
#include "meta/index/score_data.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/util/fixed_heap.h"
#include "meta/util/shim.h"

namespace meta
{
namespace index
{

const util::string_view rocchio::id = "rocchio";
const constexpr float rocchio::default_alpha;
const constexpr float rocchio::default_beta;
const constexpr uint64_t rocchio::default_k;
const constexpr uint64_t rocchio::default_max_terms;

rocchio::rocchio(std::shared_ptr<forward_index> fwd)
    : fwd_{std::move(fwd)},
      initial_ranker_{make_unique<okapi_bm25>()},
      alpha_{default_alpha},
      beta_{default_beta},
      k_{default_k},
      max_terms_{default_max_terms}
{
    // nothing
}

rocchio::rocchio(std::shared_ptr<forward_index> fwd,
                 std::unique_ptr<ranker>&& initial_ranker, float alpha,
                 float beta, uint64_t k, uint64_t max_terms)
    : fwd_{std::move(fwd)},
      initial_ranker_{std::move(initial_ranker)},
      alpha_{alpha},
      beta_{beta},
      k_{k},
      max_terms_{max_terms}
{
    // nothing
}

rocchio::rocchio(std::istream& in)
    : fwd_{[&]() {
          auto path = io::packed::read<std::string>(in);
          auto cfg = cpptoml::parse_file(path + "/config.toml");
          return make_index<forward_index>(*cfg);
      }()},
      initial_ranker_{load_ranker(in)},
      alpha_{io::packed::read<float>(in)},
      beta_{io::packed::read<float>(in)},
      k_{io::packed::read<uint64_t>(in)},
      max_terms_{io::packed::read<uint64_t>(in)}
{
    // nothing
}

void rocchio::save(std::ostream& out) const
{
    io::packed::write(out, id);
    io::packed::write(out, fwd_->index_name());
    initial_ranker_->save(out);
    io::packed::write(out, alpha_);
    io::packed::write(out, beta_);
    io::packed::write(out, k_);
    io::packed::write(out, max_terms_);
}

std::vector<search_result> rocchio::rank(ranker_context& ctx,
                                         uint64_t num_results,
                                         const filter_function_type& filter)
{
    auto fb_docs = initial_ranker_->rank(ctx, k_, filter);

    // compute the centroid in both count-space and tf-idf space
    hashing::probe_map<term_id, float> term_scores;
    hashing::probe_map<term_id, float> centroid;

    score_data sd{ctx.idx, ctx.idx.avg_doc_length(), ctx.idx.num_docs(),
                  ctx.idx.total_corpus_terms(), 1.0f};
    sd.query_term_weight = 1.0f;
    for (const auto& sr : fb_docs)
    {
        sd.d_id = sr.d_id;
        sd.doc_size = ctx.idx.doc_size(sd.d_id);
        sd.doc_unique_terms = ctx.idx.unique_terms(sd.d_id);

        auto stream = *fwd_->stream_for(sd.d_id);
        for (const auto& weight : stream)
        {
            sd.t_id = weight.first;
            sd.doc_count = ctx.idx.doc_freq(sd.t_id);
            sd.corpus_term_count = ctx.idx.total_num_occurences(sd.t_id);
            sd.doc_term_count = static_cast<uint64_t>(weight.second);

            auto& rnk = dynamic_cast<ranking_function&>(*initial_ranker_);
            term_scores[sd.t_id] += rnk.score_one(sd) / k_;
            centroid[sd.t_id] += weight.second / k_;
        }
    }

    // extract the top max_terms_ feedback terms according to their scores
    // in tf-idf space
    using scored_term = std::pair<term_id, float>;
    auto heap = util::make_fixed_heap<scored_term>(
        max_terms_, [](const scored_term& a, const scored_term& b) {
            return a.second > b.second;
        });
    for (const auto& pr : term_scores)
    {
        heap.emplace(pr.key(), pr.value());
    }

    // construct a new interpolated query in count-space from these top terms
    hashing::probe_map<term_id, float> new_query;
    for (const auto& pr : heap.extract_top())
    {
        new_query[pr.first] += beta_ * centroid[pr.first];
    }
    for (const auto& postings_ctx : ctx.postings)
    {
        new_query[postings_ctx.t_id] += alpha_ * postings_ctx.query_term_weight;
    }

    // construct a new ranker_context from the new query
    ranker_context new_ctx{ctx.idx, new_query.begin(), new_query.end(), filter};

    //  return ranking results based on the new query
    return initial_ranker_->rank(new_ctx, num_results, filter);
}

template <>
std::unique_ptr<ranker> make_ranker<rocchio>(const cpptoml::table& global,
                                             const cpptoml::table& local)
{
    auto alpha = local.get_as<double>("alpha").value_or(rocchio::default_alpha);
    auto beta = local.get_as<double>("beta").value_or(rocchio::default_beta);
    auto k = local.get_as<uint64_t>("k").value_or(rocchio::default_k);
    auto max_terms = local.get_as<uint64_t>("max-terms")
                         .value_or(rocchio::default_max_terms);

    auto init_cfg = local.get_table("feedback");
    auto f_idx = make_index<forward_index>(global);
    return make_unique<rocchio>(std::move(f_idx),
                                make_ranker(global, *init_cfg), alpha, beta,
                                k, max_terms);
}
}
}
