/**
 * @file dirichlet_prior.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_DIRICHLET_PRIOR_H_
#define META_DIRICHLET_PRIOR_H_

#include "meta/index/ranker/lm_ranker.h"
#include "meta/index/ranker/ranker_factory.h"

#include <cmath>

namespace meta
{
namespace index
{

/**
 * Implements Bayesian smoothing with a Dirichlet prior.
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "dirichlet-prior"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * mu = 2000.0
 * ~~~

 */
class dirichlet_prior : public language_model_ranker
{
  public:
    /// Identifier for this ranker.
    const static util::string_view id;

    /// Default value of mu
    const static constexpr float default_mu = 2000.0f;

    /**
     * @param mu
     */
    dirichlet_prior(float mu = default_mu);

    /**
     * Loads a dirichlet_prior ranker from a stream.
     * @param in The stream to read from
     */
    dirichlet_prior(std::istream& in);

    void save(std::ostream& out) const override;

    /**
     * Calculates the smoothed probability of a term.
     * @param sd score_data for the current query
     */
    float smoothed_prob(const score_data& sd) const override;

    /**
     * A document-dependent constant.
     * @param sd score_data for the current query
     */
    float doc_constant(const score_data& sd) const override;

    float parameter() const {
        return mu_;
    }

  protected:
    /// the Dirichlet prior parameter
//    const float mu_;
    float mu_;
};


// # TODO: choose template type instead of long
typedef long count_d;

struct docs_data
{
    // general info

    const inverted_index& idx;
    /// ids of all documents
    std::vector<doc_id> doc_ids;
    /// ids of all terms
    std::vector<term_id> term_ids;
    /// total size of documents
    count_d ref_size;
    /// C_.(n)
    std::map<count_d, count_d> docs_counts;
    /// C_k(n)
    std::map<term_id, std::map<count_d, count_d>> terms_docs_counts;
    /// vector alpha_m
    std::map<term_id, double> alpha_m;

    /**
     * Constructor to initialize most elements.
     * @param p_idx The index that is being used
     * @param p_doc_ids ids of all docs
     * @param p_term_ids ids of all terms
     */
    docs_data(const inverted_index& p_idx, std::vector<doc_id> p_doc_ids, std::vector<term_id> p_term_ids, count_d p_ref_size,
              std::map<count_d, count_d> p_docs_counts, std::map<term_id, std::map<count_d, count_d>> p_terms_docs_counts,
              std::map<term_id, double> p_alpha_m)
        : idx(p_idx), // gcc no non-const ref init from brace init list
          doc_ids{p_doc_ids},
          term_ids{p_term_ids},
          ref_size{p_ref_size},
          docs_counts{p_docs_counts},
          terms_docs_counts{p_terms_docs_counts},
          alpha_m{p_alpha_m}
    {
        /* nothing */
    }
};

class dirichlet_prior_opt : public dirichlet_prior{
public:

    dirichlet_prior_opt(float mu) : dirichlet_prior(mu) { }

    dirichlet_prior_opt(std::istream& in) : dirichlet_prior(in) { }

    template <class ForwardIterator>
    std::vector<search_result> score(inverted_index& idx, ForwardIterator begin,
                                     ForwardIterator end,
                                     uint64_t num_results = 10)
    {
        // optimize mu according to ranker_context before ranking
        this->optimize_mu(idx);

        return ranker::score(idx, begin, end, num_results);
    }

    std::map<term_id, double> get_optimized_mu(const inverted_index& idx, float eps, int max_iter) {
        return optimize_mu(idx, eps, max_iter);
    }

protected:
    inline double get_alpha(std::map<term_id, double> alpha_m){
        double alpha = 0;

        for (auto alpha_m_k: alpha_m){
            alpha += alpha_m_k.second;
        }

        return alpha;
    }

private:
    std::map<term_id, double> optimize_mu(const inverted_index& idx, float eps=1e-6, int max_iter=10000) {
        // parse idx and extract what we need
        auto docs_ids = idx.docs();
        auto terms_ids = idx.terms();

        // calculate ref_size
        count_d ref_size = 0;
        for (auto& id : docs_ids)
            ref_size += idx.doc_size(id);

        // calculate C_.(n) and C_k(n)
        std::map<count_d, count_d> docs_counts;
        std::map<term_id, std::map<count_d, count_d>> terms_docs_counts;

        long doc_size, doc_term_freq;
        for (auto d_id: docs_ids){
            doc_size = idx.doc_size(d_id);

            //// increase number of docs with the given size (C_.(n))
            docs_counts[doc_size] += 1;

            for (auto t_id: terms_ids){
                doc_term_freq = idx.term_freq(t_id, d_id);

                //// increase number of docs with the given count of word t_id (C_k(n))
                terms_docs_counts[t_id][doc_term_freq] += 1;
            }
        }

        // fill start vector alpha_m
        std::map<term_id, double> alpha_m;

        for (auto t_id: terms_ids){
            alpha_m[t_id] = idx.total_num_occurences(t_id) * default_mu;
            alpha_m[t_id] /= (double)ref_size;
        }

        // create docs_data
        docs_data dd{idx, docs_ids, terms_ids, ref_size, docs_counts, terms_docs_counts, alpha_m};

        // call optimizer
        return optimize_mu(dd, eps, max_iter);
    }

    virtual std::map<term_id, double> optimize_mu(docs_data& dd, float eps, int max_iter) = 0;
};

class dirichlet_digamma_rec: public dirichlet_prior_opt{
public:
    const static util::string_view id;

    /**
     * @param mu
     */
    dirichlet_digamma_rec(float mu = default_mu);

    /**
     * Loads a dirichlet_prior ranker from a stream.
     * @param in The stream to read from
     */
    dirichlet_digamma_rec(std::istream& in);

    void save(std::ostream& out) const override;
private:
    std::map<term_id, double> optimize_mu(docs_data& dd, float eps, int max_iter) override {
        bool all_optimized = false;
        int iter_num = 0;
        double D, S;
        double n_max = dd.docs_counts.rbegin()->first;

        // start values for alpha and alpha_m
        double alpha = default_mu, alpha_mk_new;
        std::map<term_id, double> alpha_m = dd.alpha_m;

        while (!all_optimized && iter_num < max_iter){
            D = 0.0;
            S = 0.0;
            all_optimized = true;

            alpha = get_alpha(alpha_m);

            count_d c_d;
            for (count_d n = 1; n <= n_max; n++){
                c_d = dd.docs_counts[n];

                D += 1.0/(n - 1 + alpha);
                S += c_d * D;
            }

            term_id k;
            std::map<count_d, count_d> c_k;
            double S_k;
            for (auto kv: dd.terms_docs_counts){
                k = kv.first;
                c_k = kv.second;

                D = 0.0;
                S_k = 0.0;

                count_d c_k_n, n_k_max = c_k.rbegin()->first;
                for (count_d n = 1; n <= n_k_max; n++){
                    c_k_n = c_k[n];

                    D += 1.0/(n - 1 + alpha_m[k]);
                    S_k += c_k_n * D;
                }

                alpha_mk_new = alpha_m[k] * S_k / S;

                if (std::abs(alpha_mk_new - alpha_m[k]) > eps){
                    all_optimized = false;
                }

                alpha_m[k] = alpha_mk_new;
            }

            iter_num++;
        }

        mu_ = get_alpha(alpha_m);

        return alpha_m;
    }

};

class dirichlet_log_approx: public dirichlet_prior_opt{
public:
    const static util::string_view id;

    /**
     * @param mu
     */
    dirichlet_log_approx(float mu = default_mu);

    /**
     * Loads a dirichlet_prior ranker from a stream.
     * @param in The stream to read from
     */
    dirichlet_log_approx(std::istream& in);

    void save(std::ostream& out) const override;
private:
    std::map<term_id, double> optimize_mu(docs_data& dd, float eps, int max_iter) override {
        bool all_optimized = false;
        int iter_num = 0;
        double S, S_k;
        double n_max = dd.docs_counts.rbegin()->first;

        // start values for alpha and alpha_m
        double alpha = default_mu, alpha_mk_new;
        std::map<term_id, double> alpha_m = dd.alpha_m;

        while (!all_optimized && iter_num < max_iter){
            S = 0.0;
            all_optimized = true;

            alpha = get_alpha(alpha_m);

            count_d c_d;
            // TODO: skip the zero docs counts
            for (count_d n = 1; n <= n_max; n++){
                c_d = dd.docs_counts[n];

                if (c_d != 0){
                    S += c_d * (1.0/alpha + log(n + alpha - 0.5) - log(alpha + 0.5));
                }
            }

            term_id k;
            std::map<count_d, count_d> c_k;
            for (auto kv: dd.terms_docs_counts){
                k = kv.first;
                c_k = kv.second;

                S_k = 0.0;

                count_d c_k_n, n_k_max = c_k.rbegin()->first;
                // TODO: skip the zero docs counts
                for (count_d n = 1; n <= n_k_max; n++){
                    c_k_n = c_k[n];

                    if (c_k_n != 0){
                        S_k += c_k_n * (1.0/alpha_m[k] + log(n + alpha_m[k] - 0.5) - log(alpha_m[k] + 0.5));
                    }
                }

                alpha_mk_new = alpha_m[k] * S_k / S;

                if (std::abs(alpha_mk_new - alpha_m[k]) > eps){
                    all_optimized = false;
                }

                alpha_m[k] = alpha_mk_new;
            }

            iter_num++;
        }

        mu_ = get_alpha(alpha_m);

        return alpha_m;
    }
};

class dirichlet_mackay_peto: public dirichlet_prior_opt{
public:
    const static util::string_view id;

    /**
     * @param mu
     */
    dirichlet_mackay_peto(float mu = default_mu);

    /**
     * Loads a dirichlet_prior ranker from a stream.
     * @param in The stream to read from
     */
    dirichlet_mackay_peto(std::istream& in);

    void save(std::ostream& out) const override;
private:
    std::map<term_id, double> optimize_mu(docs_data& dd, float eps, int max_iter) override {
        eps = eps;
        max_iter = max_iter;
        eps = dd.ref_size;
        mu_ = 0;
        std::map<term_id, double> alpha_m;

        return alpha_m;
    }
};

/**
 * Specialization of the factory method used to create dirichlet_prior
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<dirichlet_prior>(const cpptoml::table&);
}
}
#endif
