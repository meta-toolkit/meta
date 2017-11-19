#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "meta/embeddings/word_embeddings.h"
#include "meta/stats/statistics.h"

using namespace meta::stats;
using namespace meta::embeddings;
using namespace meta::util;

namespace meta
{
namespace stats
{
namespace opt
{

aligned_vector<std::string> get_docs_voc(aligned_vector<word_embeddings> docs_models){
    aligned_vector<std::string> docs_voc();

    for (auto m_iter: docs_models){
        // todo
    }

    return docs_voc;
}

class dirichlet_optimizer{
public:
    dirichlet_optimizer(aligned_vector<word_embeddings> docs_models, int alpha=1){
        this->docs_models_ = docs_models;
        this->default_alpha_ = alpha;

        this->docs_voc_ = get_docs_voc(docs_models);
    }

    double minka_fpi(){
        double alpha = default_alpha_;

        double nom, denom,
                alpha_m,
                alpha_dig, alpha_m_dig,
                all_words_count, k_words_count;

        // digamma(x)
        for (std::string word: voc){
            // todo
        }
    }

    double minka_newton(){
        // todo
    }

    double minka_lou(){
        // todo
    }

private:
    double minka_fpi_iters();
    double minka_newton_iters();
    double minka_lou_iters();

    aligned_vector<word_embeddings> docs_models_;
    aligned_vector<std::string> docs_voc_;

    double default_alpha_;
};

#endif // OPTIMIZATION_H
