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

#include <iostream>

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

struct docs_data
{
    // general info

    inverted_index& idx;
    /// ids of all documents
    std::vector<doc_id> doc_ids;
    /// ids of all terms
    std::vector<term_id> term_ids;

    /**
     * Constructor to initialize most elements.
     * @param p_idx The index that is being used
     * @param p_doc_ids ids of all docs
     * @param p_term_ids ids of all terms
     */
    score_data(inverted_index& p_idx, std::vector<doc_id> p_doc_ids, std::vector<doc_id> p_term_ids,
               uint64_t p_total_terms, float p_query_length)
        : idx(p_idx), // gcc no non-const ref init from brace init list
          doc_ids{p_doc_ids},
          term_ids{p_term_ids}
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

    float get_optimized_mu(const inverted_index& idx) {
        optimize_mu(idx);

        return mu_;
    }

private:
    void optimize_mu(const inverted_index& idx) {
        auto docs_ids = idx.docs();
        auto terms_ids = idx.terms();
        docs_data  dd{idx, docs_ids, terms_ids};

        optimize_mu(dd);
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

// # TODO: choose template type instead of long
typedef long count_d;

class digamma_rec: public dirichlet_prior_opt{
    void optimize_mu(const docs_data& dd, float eps=1e-6, int max_iter=100) override {
        // fill C_.(n) and C_k(n)

        std::map<count_d, count_d> docs_counts;
        std::map<term_id, std::map<count_d, count_d>> terms_docs_counts;
        long doc_size, doc_term_freq;

        for (auto d_id: dd.doc_ids){
            doc_size = dd.idx.doc_size(d_id);

            //// increase number of docs with the given size (C_.(n))
            docs_counts[doc_size] += 1;

            for (auto t_id: dd.idx.terms(d_id)){
                doc_term_freq = dd.idx.term_freq(t_id, d_id);

                //// increase number of docs with the given count of word t_id (C_k(n))
                terms_docs_counts[t_id][doc_term_freq] += 1;
            }
        }

//        // sort by ascending of occurences
//        std::sort(docs_counts.begin(), items.end());
//        for (auto key: terms_docs_counts){
//            std::sort(key.second.begin(), key.second.end());
//        }

        // p(w|REF) = dd.idx.total_num_occurences(t_id)

        // fill start vector alpha_m
        double alpha = 1;
        std::map<term_id, double> alpha_m;

        for (auto t_id: dd.idx.terms()){
            alpha_m[t_id] = dd.idx.total_num_occurences(t_id) * alpha;
        }

        double D, S;
        bool converged = false;

        while (!converged){
            D = 0;
            S = 0;

            alpha = 0;
            for (auto alpha_m_k: alpha_m){
                alpha += alpha_m_k;
            }

            count_d n, c_d;
            for (auto kv: docs_counts){
                n = kv.first;
                c_d = kv.second;

                D += 1/(n - 1 + alpha);
                S += c_d * D;
            }

            std::map<count_d, count_d> c_n;
            term_id k;
            double S_k;
            for (auto kv: terms_docs_counts){
                k = kv.first;
                c_n = kv.second;

                D = 0;
                S_k = 0;

                count_d n, c_k_n;
                for (auto kv_: c_n){
                    n = kv_.first;
                    c_k_n = kv_.second;

                    D += 1/(n - 1 + alpha * m_k);
                    S_k += c_k_n * D;
                }

                alpha_m[k] *= S_k / S;
            }
        }

    }
};

class log_approx: public dirichlet_prior_opt{
//    void optimize_mu(const inverted_index& idx) override { mu_ = 0;};
};

class mackay_peto: public dirichlet_prior_opt{
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
