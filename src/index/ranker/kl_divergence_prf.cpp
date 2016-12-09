/**
 * @file kl_divergence_prf.cpp
 * @author Chase Geigle
 */

#include <stdexcept>

#include "cpptoml.h"
#include "meta/index/ranker/dirichlet_prior.h"
#include "meta/index/ranker/kl_divergence_prf.h"
#include "meta/index/ranker/unigram_mixture.h"
#include "meta/index/score_data.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/util/fixed_heap.h"
#include "meta/util/iterator.h"
#include "meta/util/shim.h"

namespace meta
{
namespace index
{

const util::string_view kl_divergence_prf::id = "kl-divergence-prf";
const constexpr float kl_divergence_prf::default_alpha;
const constexpr float kl_divergence_prf::default_lambda;
const constexpr uint64_t kl_divergence_prf::default_k;
const constexpr uint64_t kl_divergence_prf::default_max_terms;

kl_divergence_prf::kl_divergence_prf(std::shared_ptr<forward_index> fwd)
    : fwd_{std::move(fwd)},
      initial_ranker_{make_unique<dirichlet_prior>()},
      alpha_{default_alpha},
      lambda_{default_lambda},
      k_{default_k},
      max_terms_{default_max_terms}
{
    // nothing
}

kl_divergence_prf::kl_divergence_prf(
    std::shared_ptr<forward_index> fwd,
    std::unique_ptr<language_model_ranker>&& initial_ranker, float alpha,
    float lambda, uint64_t k, uint64_t max_terms)
    : fwd_{std::move(fwd)},
      initial_ranker_{std::move(initial_ranker)},
      alpha_{alpha},
      lambda_{lambda},
      k_{k},
      max_terms_{max_terms}
{
    // nothing
}

kl_divergence_prf::kl_divergence_prf(std::istream& in)
    : fwd_{[&]() {
          auto path = io::packed::read<std::string>(in);
          auto cfg = cpptoml::parse_file(path + "/config.toml");
          return make_index<forward_index>(*cfg);
      }()},
      initial_ranker_{load_lm_ranker(in)},
      alpha_{io::packed::read<float>(in)},
      lambda_{io::packed::read<float>(in)},
      k_{io::packed::read<uint64_t>(in)},
      max_terms_{io::packed::read<uint64_t>(in)}
{
    // nothing
}

void kl_divergence_prf::save(std::ostream& out) const
{
    io::packed::write(out, id);
    io::packed::write(out, fwd_->index_name());
    initial_ranker_->save(out);
    io::packed::write(out, alpha_);
    io::packed::write(out, lambda_);
    io::packed::write(out, k_);
    io::packed::write(out, max_terms_);
}

std::vector<search_result>
kl_divergence_prf::rank(ranker_context& ctx, uint64_t num_results,
                        const filter_function_type& filter)
{
    auto fb_docs = initial_ranker_->rank(ctx, k_, filter);
    auto extract_docid = [](const search_result& sr) { return sr.d_id; };

    // construct feedback document set
    learn::dataset fb_dset{
        fwd_, util::make_transform_iterator(fb_docs.begin(), extract_docid),
        util::make_transform_iterator(fb_docs.end(), extract_docid),
        printing::no_progress_trait{}};

    // learn the feedback model using the EM algorithm
    feedback::training_options options;
    options.lambda = lambda_;
    auto fb_model = feedback::unigram_mixture(
        [&](term_id tid) {
            float term_count = ctx.idx.total_num_occurences(tid);
            return term_count / ctx.idx.total_corpus_terms();
        },
        fb_dset, options);

    // extract only the top max_terms from the feedback model
    using scored_term = std::pair<term_id, float>;
    auto heap = util::make_fixed_heap<scored_term>(
        max_terms_, [&](const scored_term& a, const scored_term& b) {
            return a.second > b.second;
        });
    fb_model.each_seen_event(
        [&](term_id tid) { heap.emplace(tid, fb_model.probability(tid)); });

    // interpolate the old query with the top terms from the feedback model
    hashing::probe_map<term_id, float> new_query;
    for (const auto& pr : heap.extract_top())
    {
        new_query[pr.first] += alpha_ * pr.second;
    }
    for (const auto& postings_ctx : ctx.postings)
    {
        auto p_wq = postings_ctx.query_term_weight / ctx.query_length;
        new_query[postings_ctx.t_id] += (1.0f - alpha_) * p_wq;
    }

    // construct a new ranker_context from the new query
    ranker_context new_ctx{ctx.idx, new_query.begin(), new_query.end(), filter};

    // return ranking results based on the new query
    return initial_ranker_->rank(new_ctx, num_results, filter);
}

template <>
std::unique_ptr<ranker>
make_ranker<kl_divergence_prf>(const cpptoml::table& global,
                               const cpptoml::table& local)
{
    if (global.begin() == global.end())
    {
        LOG(fatal) << "Global configuration group was empty in construction of "
                      "kl_divergence_prf ranker"
                   << ENDLG;
        LOG(fatal) << "Did you mean to call index::make_ranker(global, local) "
                      "instead of index::make_ranker(local)?"
                   << ENDLG;
        throw ranker_exception{"empty global configuration provided to "
                               "construction of kl_divergence_prf ranker"};
    }

    auto alpha = local.get_as<double>("alpha").value_or(
        kl_divergence_prf::default_alpha);
    auto lambda = local.get_as<double>("lambda").value_or(
        kl_divergence_prf::default_lambda);
    auto k = local.get_as<uint64_t>("k").value_or(kl_divergence_prf::default_k);
    auto max_terms = local.get_as<uint64_t>("max-terms")
                         .value_or(kl_divergence_prf::default_max_terms);
    auto init_cfg = local.get_table("feedback");
    auto f_idx = make_index<forward_index>(global);
    return make_unique<kl_divergence_prf>(std::move(f_idx),
                                          make_lm_ranker(global, *init_cfg),
                                          alpha, lambda, k, max_terms);
}
}
}
