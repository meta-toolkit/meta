/**
 * @file emd.h
 * @author lolik111
 */

#ifndef META_EMD_H
#define META_EMD_H

#include "meta/caching/all.h"
#include "meta/hashing/hash.h"
#include "meta/math/vector.h"
#include "meta/parallel/algorithm.h"
#include "meta/util/min_cost_flow.h"
#include "meta/util/range.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

namespace meta
{

namespace index
{

class Document
{
  public:
    size_t n_terms;
    std::vector<size_t> ids;
    std::vector<double> weights;
};

class em_distance
{
  public:
    using metric_type
        = std::function<double(const util::array_view<const double>&,
                               const util::array_view<const double>&)>;

    em_distance(
        std::
            shared_ptr<caching::dblru_shard_cache<std::pair<uint64_t, uint64_t>,
                                                  double>>
                cache_,
        std::shared_ptr<embeddings::word_embeddings> embeddings,
        std::string algorithm_type, metric_type metric, size_t nthreads = 1)
        : nthreads_(nthreads),
          cache_(cache_),
          embeddings_(embeddings),
          algorithm_type_(algorithm_type),
          dimension_(embeddings->vector_size()),
          dist(metric)
    {
        methods_.emplace("rwmd",
                         [this](const Document& doc1, const Document& doc2) {
                             auto score1 = this->emd_relaxed(doc1, doc2);
                             auto score2 = this->emd_relaxed(doc2, doc1);
                             return std::max(score1, score2);
                         });
        methods_.emplace("wcd",
                         [this](const Document& doc1, const Document& doc2) {
                             return this->wcd(doc1, doc2);
                         });
        methods_.emplace("emd",
                         [this](const Document& doc1, const Document& doc2) {
                             return this->emd(doc1, doc2);
                         });
    }

    double score(const Document& doc1, const Document& doc2)
    {
        return methods_[algorithm_type_](doc1, doc2);
    }

    double emd(const Document& doc1, const Document& doc2)
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
                double dist = f_c_distance(doc1, doc2, i, j);
                assert(dist >= 0);
                cost[i][j + doc1.n_terms] = dist;
                cost[j + doc1.n_terms][i] = dist;
            }
        }
        util::min_cost_flow<double> mcf;
        auto score = mcf.emd_hat(supply, demand, cost);

        return score;
    }

    double emd_relaxed(const Document& doc1, const Document& doc2)
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
                distance[j] = f_c_distance(doc1, doc2, i, j);
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

    double wcd(const Document& doc1, const Document& doc2)
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

    static double l2diff_norm(const util::array_view<const double>& a,
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

    static double cosine(const util::array_view<const double>& a,
                         const util::array_view<const double>& b)
    {
        if (a.begin() == b.begin())
            return 0;
        return (1.0 - std::inner_product(a.begin(), a.end(), b.begin(), 0.0))
               / 2.0;
    }

  private:
    const size_t nthreads_;
    std::shared_ptr<caching::dblru_shard_cache<std::pair<uint64_t, uint64_t>,
                                               double>>
        cache_;
    std::shared_ptr<embeddings::word_embeddings> embeddings_;
    const std::string algorithm_type_;
    const size_t dimension_;
    const metric_type dist;

    std::unordered_map<std::string,
                       std::function<double(const Document&, const Document&)>>
        methods_;

    double f_c_distance(const Document& doc1, const Document& doc2,
                        size_t first, size_t second)
    {
        std::pair<size_t, size_t> pair;
        if (doc1.ids[first] < doc2.ids[second])
        {
            pair = {doc1.ids[first], doc2.ids[second]};
        }
        else
        {
            pair = {doc2.ids[second], doc1.ids[first]};
        }

        auto val = cache_->find(pair);

        if (!val)
        {
            val = dist(embeddings_->at(doc1.ids[first]),
                       embeddings_->at(doc2.ids[second]));
            cache_->insert(pair, val.value());
        }
        return val.value();
    }
};
}
}

#endif // META_EMD_H
