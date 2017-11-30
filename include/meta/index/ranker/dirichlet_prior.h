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
#include <iostream>

using namespace std;

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

    /**
     * Constructor to initialize most elements.
     * @param p_idx The index that is being used
     * @param p_doc_ids ids of all docs
     * @param p_term_ids ids of all terms
     */
    docs_data(const inverted_index& p_idx, std::vector<doc_id> p_doc_ids, std::vector<term_id> p_term_ids, count_d p_ref_size)
        : idx(p_idx), // gcc no non-const ref init from brace init list
          doc_ids{p_doc_ids},
          term_ids{p_term_ids},
          ref_size{p_ref_size}
    {
        /* nothing */
    }
};

class dirichlet_prior_opt : public dirichlet_prior{
public:
    template <class ForwardIterator>
    std::vector<search_result> score(inverted_index& idx, ForwardIterator begin,
                                     ForwardIterator end,
                                     uint64_t num_results = 10)
    {
        // optimize mu according to ranker_context before ranking
        this->optimize_mu(idx);

        return ranker::score(idx, begin, end, num_results);
    }

    float get_optimized_mu(const inverted_index& idx, float eps=1e-6, int max_iter=100) {
        optimize_mu(idx, eps, max_iter);

        return mu_;
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
    void optimize_mu(const inverted_index& idx, float eps=1e-6, int max_iter=100) {
        auto docs_ids = idx.docs();
        auto terms_ids = idx.terms();

        count_d ref_size = 0;
        for (auto& id : docs_ids)
            ref_size += idx.doc_size(id);

        docs_data dd{idx, docs_ids, terms_ids, ref_size};

        optimize_mu(dd, eps, max_iter);
//        std::cout << idx.unique_terms() << std::endl;

//        for (auto d_id: docs_ids){
//            for (auto t_id: terms_ids){
//                std::cout << idx.term_freq(t_id, d_id) << std::endl;
//            }
//        }

//        optimize_mu(std::vector<doc_id> docs_ids,
//        idx.unique_terms()
//        idx.total_corpus_terms()

    }

    virtual void optimize_mu(const docs_data& dd, float eps=1e-6, int max_iter=100) = 0;
};

class dirichlet_digamma_rec: public dirichlet_prior_opt{
    void optimize_mu(const docs_data& dd, float eps=1e-6, int max_iter=100) override {
        // fill C_.(n) and C_k(n)

        std::map<count_d, count_d> docs_counts;
        std::map<term_id, std::map<count_d, count_d>> terms_docs_counts;
        long doc_size, doc_term_freq;

        cout << "Docs and terms:\n";
        for (auto d_id: dd.doc_ids){
            doc_size = dd.idx.doc_size(d_id);

            //// increase number of docs with the given size (C_.(n))
            docs_counts[doc_size] += 1;

            cout << d_id << " " << doc_size << " " << docs_counts[doc_size] << endl;
            for (auto t_id: dd.term_ids){
                doc_term_freq = dd.idx.term_freq(t_id, d_id);

                //// increase number of docs with the given count of word t_id (C_k(n))
                terms_docs_counts[t_id][doc_term_freq] += 1;

                cout << "    " << t_id << " " << doc_term_freq << " " << terms_docs_counts[t_id][doc_term_freq] << endl;
            }
        }

        cout << "\nDocuments_ids count: " << dd.doc_ids.size() << "; Terms ids count: " << dd.term_ids.size() << endl;

        cout << "\nDocuments sizes frequency:\n";
        for (auto kv: docs_counts){
            cout << kv.first << " " << kv.second << endl;
        }

        int occur_sum, freq_sum;
        cout << "\nTerms frequency in each doc:\n";
        for (auto kv: terms_docs_counts){
            occur_sum = 0;
            freq_sum = 0;
            cout << dd.idx.total_num_occurences(kv.first) << " " << kv.first << endl;
            for (auto kv_: kv.second){
                occur_sum += kv_.second;
                freq_sum += kv_.first * kv_.second;
                cout << "   " << kv_.first << " " << kv_.second << endl;
            }
            cout << "    " << freq_sum << " " << occur_sum << " total occurences" << endl;
        }

//        // sort by ascending of occurences
//        std::sort(docs_counts.begin(), items.end());
//        for (auto key: terms_docs_counts){
//            std::sort(key.second.begin(), key.second.end());
//        }

        // p(w|REF) = dd.idx.total_num_occurences(t_id)

        // fill start vector alpha_m
        double alpha = 2000.0, alpha_mk_new;
        std::map<term_id, double> alpha_m;

        cout << "\nStart alpha: ";
        for (auto t_id: dd.term_ids){
            alpha_m[t_id] = dd.idx.total_num_occurences(t_id) * alpha;
            alpha_m[t_id] /= (double)dd.ref_size;
            cout << alpha_m[t_id] << " ";
        }

        double D, S;
        bool all_optimized = false;
        int iter_num = 0;

        double n_max = docs_counts.rbegin()->first;
        cout << "\n n_max=" << n_max << endl;

        while (!all_optimized && iter_num < max_iter){
            D = 0.0;
            S = 0.0;
            all_optimized = true;

            alpha = get_alpha(alpha_m);

            cout << "\nIter " << iter_num << " alpha = " << alpha;
            count_d c_d;
            for (count_d n = 1; n <= n_max; n++){
                c_d = docs_counts[n];

                D += 1.0/(n - 1 + alpha);
                S += c_d * D;
            }

            cout << "\nD = " << D << "; S = " << S << "; S_k = ";

            std::map<count_d, count_d> c_k;
            term_id k;
            double S_k;
            for (auto kv: terms_docs_counts){
                k = kv.first;
                c_k = kv.second;

                D = 0.0;
                S_k = 0.0;

                count_d c_k_n, n_k_max = c_k.rbegin()->first;
                cout << "\n n_k_max=" << n_k_max << endl;
                for (count_d n = 1; n <= n_k_max; n++){
                    c_k_n = c_k[n];

                    D += 1.0/(n - 1 + alpha_m[k]);
                    S_k += c_k_n * D;
                }

                cout << S_k << " ";

                alpha_mk_new = alpha_m[k] * S_k / S;

                if (std::abs(alpha_mk_new - alpha_m[k]) > eps){
                    all_optimized = false;
                }

                alpha_m[k] = alpha_mk_new;
            }

            cout << "\nVector alpha_m after the iter: ";
            for (auto kv: alpha_m){
                cout << " " << kv.second;
            }

            iter_num++;
        }

        mu_ = get_alpha(alpha_m);
        cout << endl << mu_ << endl;
    }
};

class dirichlet_log_approx: public dirichlet_prior_opt{
//    void optimize_mu(const inverted_index& idx) override { mu_ = 0;};
};

class dirichlet_mackay_peto: public dirichlet_prior_opt{
//    void optimize_mu(const inverted_index& idx) override { mu_ = 0;};
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
