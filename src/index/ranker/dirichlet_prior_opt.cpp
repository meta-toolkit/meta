/**
 * @file dirichlet_prior_opt.cpp
 * @author Aleksey Marashov, Kolomiets Maksim
 */

#include "cpptoml.h"
#include "meta/index/ranker/dirichlet_prior_opt.h"
#include "meta/index/score_data.h"

namespace meta
{
namespace index
{

// makers

const util::string_view dirichlet_digamma_rec::id = "dirichlet-digamma-rec";
template <>
std::unique_ptr<ranker>
    make_ranker<dirichlet_digamma_rec>(const cpptoml::table& config)
{
    auto mu = config.get_as<double>("mu").value_or(dirichlet_digamma_rec::default_mu);
    if (mu < 0)
        throw ranker_exception{"dirichlet-digamma-rec mu must be >= 0"};
    return make_unique<dirichlet_digamma_rec>(mu);
}

const util::string_view dirichlet_log_approx::id = "dirichlet-log-approx";
template <>
std::unique_ptr<ranker>
    make_ranker<dirichlet_log_approx>(const cpptoml::table& config)
{
    auto mu = config.get_as<double>("mu").value_or(dirichlet_log_approx::default_mu);
    if (mu < 0)
        throw ranker_exception{"dirichlet-log-approx mu must be >= 0"};
    return make_unique<dirichlet_log_approx>(mu);
}

const util::string_view dirichlet_mackay_peto::id = "dirichlet-mackay-peto";
template <>
std::unique_ptr<ranker>
    make_ranker<dirichlet_mackay_peto>(const cpptoml::table& config)
{
    auto mu = config.get_as<double>("mu").value_or(dirichlet_mackay_peto::default_mu);
    if (mu < 0)
        throw ranker_exception{"dirichlet-mackay-peto mu must be >= 0"};
    return make_unique<dirichlet_mackay_peto>(mu);
}

// constructors

dirichlet_digamma_rec::dirichlet_digamma_rec(float mu) : dirichlet_prior_opt(mu)
{
    // nothing
}

dirichlet_digamma_rec::dirichlet_digamma_rec(std::istream& in)
    : dirichlet_prior_opt(in)
{
    // nothing
}

void dirichlet_digamma_rec::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, mu_);
}


dirichlet_log_approx::dirichlet_log_approx(float mu) : dirichlet_prior_opt(mu)
{
    // nothing
}

dirichlet_log_approx::dirichlet_log_approx(std::istream& in)
    : dirichlet_prior_opt(in)
{
    // nothing
}

void dirichlet_log_approx::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, mu_);
}


dirichlet_mackay_peto::dirichlet_mackay_peto(float mu) : dirichlet_prior_opt(mu)
{
    // nothing
}

dirichlet_mackay_peto::dirichlet_mackay_peto(std::istream& in)
    : dirichlet_prior_opt(in)
{
    // nothing
}

void dirichlet_mackay_peto::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, mu_);
}

// optimization methods

std::map<term_id, double> dirichlet_digamma_rec::optimize_mu(docs_data& dd, float eps, int max_iter) {
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

std::map<term_id, double> dirichlet_log_approx::optimize_mu(docs_data& dd, float eps, int max_iter) {
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

std::map<term_id, double> dirichlet_mackay_peto::optimize_mu(docs_data& dd, float eps, int max_iter) {
    bool all_optimized = false;
    int iter_num = 0;

    // start values for alpha and alpha_m
    double alpha = default_mu, alpha_mk_new;
    std::map<term_id, double> alpha_m = dd.alpha_m;

    while (!all_optimized && iter_num < max_iter){
        all_optimized = true;

        alpha = get_alpha(alpha_m);

        // compute K(alpha)
        double K_alpha = 0;
        for (auto d_id: dd.doc_ids){
            double n_d = dd.idx.doc_size(d_id);
            K_alpha += log((n_d + alpha) / alpha) + 0.5 * n_d / (alpha * (n_d + alpha));
        }

        term_id k;
        std::map<count_d, count_d> c_k;
        for (auto kv: dd.terms_docs_counts){
            k = kv.first;
            c_k = kv.second;

            count_d n_k_max = c_k.rbegin()->first;

            // compute V_k
            count_d V_k = dd.idx.doc_freq(k);

            // compute H_k and G_k
            double H_k = 0, G_k = 0;
            count_d N_f = 0;
            for (count_d f = n_k_max; f >= 2; f--){
                N_f += c_k[f];

                G_k += (double) N_f / (f - 1.0);
                H_k += (double) N_f / pow(f - 1.0, 2);
            }

            // recompute alpha_mk
            alpha_mk_new = 2 * V_k / (K_alpha - G_k + sqrt(pow(K_alpha - G_k, 2) + 4 * H_k * V_k));

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

}
}
