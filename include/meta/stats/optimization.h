#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "meta/embeddings/word_embeddings.h"
#include "meta/stats/statistics.h"
#include "meta/analyzers/featurizer.h"

#include <cmath>

using namespace meta::stats;
using namespace meta::embeddings;
using namespace meta::util;
using namespace meta::analyzers;

namespace meta
{
namespace stats
{
namespace opt
{

aligned_vector<long> get_docs_sizes(aligned_vector<feature_map<long>> docs_models){
    aligned_vector<long> docs_sizes;

    long doc_size;
    for (int i = 0; i < docs_models.size(); i++){
        doc_size = 0;

        for (auto word: docs_models[i]){
            doc_size += docs_models[i][word];
        }

        docs_sizes.push_back(doc_size);
    }

    return docs_sizes;
}

feature_map<long> get_ref_voc(aligned_vector<feature_map<long>> docs_models){
    feature_map<long> ref_voc;
    featurizer<long> f(ref_voc);

    for (feature_map<long> doc_model: docs_models){
        for (auto word: doc_model){
            f(word, doc_model[word]);
        }
    }

    return ref_voc;
}

class dirichlet_optimizer{
public:
    dirichlet_optimizer(aligned_vector<feature_map<long>> docs_models, int alpha=1){
        this->docs_models_ = docs_models;
        this->docs_sizes_ = get_docs_sizes(docs_models);

        this->default_alpha_ = alpha;

        this->ref_voc_ = get_docs_voc(docs_models);
    }

    typedef std::map<std::string, double> text_vector;

    text_vector minka_fpi(double eps=1e-6, int max_iters=100){
        std::map<std::string, double> alpha_m;

        // create initial alpa_m vector
        for (auto word: ref_voc_){
            alpha_m[word] = default_alpha_ * ref_voc_[word];
        }

        // stoping criteria for the whole vector alpha_m
        int vector_iteration = 0;
        double l_dist = std::numeric_limits::infinity();
        bool all_optimal = true;

        while (vector_iteration <= max_iters && !all_optimal){
            all_optimal = true;
            std::string word_k;
            double alpha_m_k, alpha_k, alpha_m_k_new;

            for (auto alpha_m_iter: alpha_m){
                word_k = alpha_m_iter.first;
                alpha_m_k = alpha_m_iter.second;

                alpha_k = alpha_m_k / ref_voc_[word_k];

                // make a step and find new alpha_m_k
                alpha_m_k_new = minka_fpi_step(word_k, alpha_k, alpha_m_k);

                if (!is_optimal(alpha_m_k, alpha_m_k_new)){
                    all_optimal = false;

                    alpha_m[word_k] = alpha_m_k_new;
                }
            }
        }

        return alpha_m;
    }

    double minka_newton(){
        // todo
    }

    double minka_lou(){
        // todo
    }

private:
    double minka_fpi_step(std::string word_k, double alpha_k, double alpha_m_k){
        double nom = 0, denom = 0;

        double alpha_m_k_dig = digamma(alpha_m_k),
                alpha_k_dig = digamma(alpha_k);

        long all_words_count, k_words_count;

        for (int d = 0; d < docs_models_.size(); d++){
            nom += digamma(docs_models_[d][word_k] + alpha_m_k) - alpha_m_k_dig;

            denom += digamma(docs_sizes_[d] + alpha_k) - alpha_k_dig;
        }

        return alpha_m_k * nom / denom;
    }

    double minka_newton_iters();
    double minka_lou_iters();

    aligned_vector<feature_map<long>> docs_models_;
    aligned_vector<long> docs_sizes_;

    feature_map<long> ref_voc_;

    double default_alpha_;
};

#endif // OPTIMIZATION_H
