/**
 * @file wm_distance.cpp
 * @author lolik111
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

//#include <algorithm>
//#include <cmath>
//#include <cstdint>
//#include <numeric>

#include "meta/embeddings/wmd/wm_distance.h"
#include "meta/embeddings/wmd/min_cost_flow.h"
#include "meta/parallel/algorithm.h"

namespace meta
{

namespace embeddings
{

wm_distance::wm_distance(
    std::shared_ptr<caching::dblru_shard_cache<std::pair<uint64_t, uint64_t>,
                                               double>>
        cache_,
    std::shared_ptr<embeddings::word_embeddings> embeddings, metric_type metric,
    size_t nthreads /*= 1*/)
    : nthreads_(nthreads),
      cache_(cache_),
      embeddings_(embeddings),
      dimension_(embeddings->vector_size()),
      dist(metric)
{
    methods_.emplace(
        "rwmd", [this](const emb_document& doc1, const emb_document& doc2) {
            auto score1 = this->emd_relaxed(doc1, doc2);
            auto score2 = this->emd_relaxed(doc2, doc1);
            return std::max(score1, score2);
        });
    methods_.emplace(
        "wcd", [this](const emb_document& doc1, const emb_document& doc2) {
            return this->wcd(doc1, doc2);
        });
    methods_.emplace(
        "emd", [this](const emb_document& doc1, const emb_document& doc2) {
            return this->emd(doc1, doc2);
        });
}

double wm_distance::score(const std::string algorithm_type,
                          const emb_document& doc1, const emb_document& doc2)
{
    return methods_[algorithm_type](doc1, doc2);
}

double wm_distance::emd(const emb_document& doc1, const emb_document& doc2)
{
    std::vector<double> supply(doc1.n_terms + doc2.n_terms, 0);
    std::vector<double> demand(doc1.n_terms + doc2.n_terms, 0);

    for (size_t i = 0; i < doc1.n_terms; ++i)
    {
        supply[i] = doc1.weights[i];
    }

    for (size_t i = 0; i < doc2.n_terms; ++i)
    {
        demand[doc1.n_terms + i] = doc2.weights[i];
    }

    std::vector<std::vector<double>> cost(
        supply.size(), std::vector<double>(supply.size(), 0));

    for (size_t i = 0; i < doc1.n_terms; ++i)
    {
        for (size_t j = 0; j < doc2.n_terms; ++j)
        {
            double dist = f_c_distance(doc1.ids[i], doc2.ids[j]);
            assert(dist >= 0);
            cost[i][j + doc1.n_terms] = dist;
            cost[j + doc1.n_terms][i] = dist;
        }
    }
    embeddings::min_cost_flow<double> mcf;
    auto score = mcf.emd_hat(supply, demand, cost);

    return score;
}

double wm_distance::emd_relaxed(const emb_document& doc1,
                                const emb_document& doc2)
{
    std::vector<uint64_t> ids(doc2.n_terms);
    for (size_t i = 0; i < doc2.n_terms; i++)
    {
        ids[i] = i;
    }

    double acc = 0;
    for (size_t i = 0; i < doc1.n_terms; i++)
    {
        std::vector<double> distance(doc2.n_terms);
        for (size_t j = 0; j < doc2.n_terms; ++j)
        {
            distance[j] = f_c_distance(doc1.ids[i], doc2.ids[j]);
        }

        if (doc1.weights[i] != 0)
        {
            std::sort(ids.begin(), ids.end(),
                      [&](const size_t a, const size_t b) {
                          bool ans;
                          ans = distance[a] < distance[b];
                          return ans;
                      });

            double remaining = doc1.weights[i];
            for (size_t j = 0; j < doc2.n_terms; j++)
            {
                uint64_t w = ids[j];
                if (remaining < doc2.weights[w])
                {
                    acc += remaining * distance[w];
                    break;
                }
                else
                {
                    remaining -= doc2.weights[w];
                    acc += doc2.weights[w] * distance[w];
                }
            }
        }
    }
    return acc;
}

double wm_distance::wcd(const emb_document& doc1, const emb_document& doc2)
{
    using namespace meta::math::operators;

    std::vector<double> res1(dimension_, 0);
    std::vector<double> res2(dimension_, 0);

    auto start = doc1.ids.begin();
    for (auto w1 : doc1.weights)
    {
        res1 = res1 + embeddings_->at(*start++) * w1;
    }

    start = doc2.ids.begin();
    for (auto w2 : doc2.weights)
    {
        res2 = res2 + embeddings_->at(*start++) * w2;
    }

    return dist(res1, res2);
}

double wm_distance::l2diff_norm(const util::array_view<const double>& a,
                                const util::array_view<const double>& b)
{
    double res = 0.0;
    auto it1 = a.begin();
    auto it2 = b.begin();
    if (it1 == it2)
    {
        return 0;
    }

    while (it1 != a.end())
    {
        double val = *it1 - *it2;
        res += val * val;
        it1++;
        it2++;
    }

    return res;
}

double wm_distance::cosine(const util::array_view<const double>& a,
                           const util::array_view<const double>& b)
{
    if (a.begin() == b.begin())
        return 0;
    return (1.0 - std::inner_product(a.begin(), a.end(), b.begin(), 0.0)) / 2.0;
}

double wm_distance::f_c_distance(const size_t first_word_id,
                                 const size_t second_word_id)
{
    std::pair<size_t, size_t> pair;
    if (first_word_id < second_word_id)
    {
        pair = {first_word_id, second_word_id};
    }
    else
    {
        pair = {second_word_id, first_word_id};
    }

    auto val = cache_->find(pair);

    return val.value_or([&]() {
        auto dst = dist(embeddings_->at(first_word_id),
                        embeddings_->at(second_word_id));
        cache_->insert(pair, dst);
        return dst;
    }());
}
}
}
