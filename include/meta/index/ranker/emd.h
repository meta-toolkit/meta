/**
 * @file emd.h
 * @author lolik111
 */

#ifndef META_EMD_H
#define META_EMD_H


#include <thread>
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <functional>
#include "meta/parallel/algorithm.h"
#include "meta/math/vector.h"
#include "meta/caching/all.h"
#include "meta/util/range.h"
#include "meta/hashing/hash.h"
#include "meta/util/min_cost_flow.h"


namespace meta
{

namespace index
{

class Document
{
public:
    size_t n_terms;
    std::vector<size_t> ids;
    std::vector<std::vector<double>> vectors;
    std::vector<double> weights;
};


class em_distance
{
public:
    using metric_type = std::function<double(const std::vector<double>&, const
    std::vector<double>&)>;

    em_distance(const std::shared_ptr<caching::dblru_shard_cache
            <std::pair<uint64_t, uint64_t>, double>> &cache_,
                metric_type metric,
                size_t dimension,
                size_t nthreads = std::thread::hardware_concurrency())
            : nthreads_(nthreads), cache_(cache_), dimension_(dimension),
              dist(metric)
    {
    }

    void fill(){
        auto f = [this](const Document &doc1, const Document &doc2){
            return this->emd_relaxed(doc1, doc2);
        };
        std::function<double(const Document&, const Document&)> fz = f;
        methods_.insert(std::make_pair(std::string("rwmd"), fz));

    }


    double emd(Document &doc1, Document &doc2)
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

        std::vector<std::vector<double>> cost(supply.size(),
                                              std::vector<double>(supply.size(), 0));

        auto f_c_distance = [&](size_t first, size_t second)
        {
            std::pair<size_t, size_t> pair;
            if (doc1.ids[first] < doc2.ids[second])
            {
                pair = std::make_pair(doc1.ids[first],
                                      doc2.ids[second]);
            } else {
                pair = std::make_pair(doc2.ids[second],
                                      doc1.ids[first]);
            }

            auto val = cache_->find(pair);

            if(!val)
            {
                val = dist(doc1.vectors[first],
                           doc2.vectors[second]);
                cache_->insert(pair, val.value());
            }
            return val.value();
        };

        for (size_t i = 0; i < doc1.n_terms; ++i)
        {
            for (size_t j = 0; j < doc2.n_terms; ++j)
            {
                double dist = f_c_distance(i, j);
                assert(dist >= 0);
                cost[i][j + doc1.n_terms] = dist;
                cost[j + doc1.n_terms][i] = dist;
            }
        }
        util::min_cost_flow<double> mcf;
        auto score = mcf.emd_hat(supply, demand, cost);

        return score;
    }

    double forward_emd(Document &doc1, Document &doc2){
//        std::vector<double> supply;
//        std::unordered_map<size_t, size_t> check_map;
//
//        for (size_t i = 0; i < doc1.n_terms; ++i)
//        {
//            check_map.insert({doc1.ids[i], i});
//        }
//
//        for (size_t i = 0; i < doc2.n_terms; ++i)
//        {
//            std::unordered_map<size_t, size_t>::iterator ind;
//            if((ind = check_map.find(doc2.ids[i])) != check_map.end()){
//                auto k = ind->second;
//                if (doc1.weights[k] < doc2.weights[i]){
//                    doc2.weights[i] -= doc1.weights[k];
//                    doc1.weights[k] = 0;
//                } else {
//                    doc1.weights[k] -= doc2.weights[i];
//                    doc2.weights[i] = 0;
//                }
//            }
//        }
//
//
//        for (size_t i = 0; i < doc1.n_terms; ++i)
//        {
//            if(doc1.weights[i] != 0)
//                supply.push_back(doc1.weights[i]);
//        }
//        std::vector<double> demand(supply.size(), 0);
//
//        for (size_t i = 0; i < doc2.n_terms; ++i)
//        {
//            if(doc2.weights[i] != 0)
//                demand.push_back(doc2.weights[i]);
//        }
//        supply.resize(demand.size(), 0);
        std::vector<double> supply(doc1.n_terms + doc2.n_terms, 0);
        std::vector<double> demand(doc1.n_terms + doc2.n_terms, 0);
        std::vector<double> xtra(doc1.n_terms + doc2.n_terms, 0);


        for (size_t i = 0; i < doc1.n_terms; ++i)
        {
            supply[i] = doc1.weights[i];
            xtra[i] = doc1.weights[i];
        }

        for (size_t i = 0; i < doc2.n_terms; ++i)
        {
            demand[doc1.n_terms + i] = doc2.weights[i];
            xtra[doc1.n_terms + i] = -doc2.weights[i];
        }

        std::vector<std::vector<double>> cost(supply.size(),
                                              std::vector<double>(supply.size(), 0));

        auto f_c_distance = [&](size_t first, size_t second)
        {
            std::pair<size_t, size_t> pair;
            if (doc1.ids[first] < doc2.ids[second])
            {
                pair = std::make_pair(doc1.ids[first],
                                      doc2.ids[second]);
            } else {
                pair = std::make_pair(doc2.ids[second],
                                      doc1.ids[first]);
            }

            auto val = cache_->find(pair);

            if(!val)
            {
                val = dist(doc1.vectors[first],
                           doc2.vectors[second]);
                cache_->insert(pair, val.value());
            }
            return val.value();
        };

        std::vector<std::list<util::edge<double>>> edges;

        for (size_t i = 0; i < doc1.n_terms; ++i)
        {
            std::list<util::edge<double>> list;
            for (size_t j = 0; j < doc2.n_terms; ++j)
            {
                double dist = f_c_distance(i, j);
                list.push_back({doc1.n_terms + j, dist});

                assert(dist >= 0);
                cost[i][j + doc1.n_terms] = dist;
                cost[j + doc1.n_terms][i] = dist;
            }
            edges.push_back(list);
        }

        for (size_t i = 0; i < doc2.n_terms; ++i)
        {
            std::list<util::edge<double>> list;
            edges.push_back(list);
        }

        util::min_cost_flow<double> mcf;
        std::vector<std::list<util::edge_weighted<double>>> f(xtra.size());

        auto score = mcf.compute_min_cost_flow(xtra, edges, f);

        return score;

    }

    double emd_relaxed2(Document &doc1, Document &doc2)
    {
        std::vector<uint64_t> boilerplate(doc2.n_terms);
        for (size_t i = 0; i < doc2.n_terms; i++) {
            boilerplate[i] = i;
        }

        double acc = 0;
        for (size_t i = 0; i < doc1.n_terms; i++) {

            if (doc1.weights[i] != 0) {
                std::sort(
                        boilerplate.begin(),
                        boilerplate.end(),
                        [&](const int a, const int b){
                            bool ans;
                            ans = dist(doc1.vectors[i], doc2.vectors[a]) <
                                  dist(doc1.vectors[i], doc2.vectors[b]);
                            return ans;
                        });

                double remaining = doc1.weights[i];
                for (size_t j = 0; j < doc2.n_terms; j++) {
                    uint64_t w = boilerplate[j];
                    if (remaining < doc2.weights[w]) {
                        acc += remaining *
                               dist(doc1.vectors[i], doc2.vectors[w]);
                        break;
                    } else {
                        remaining -= doc2.weights[w];
                        acc += doc2.weights[w] *
                               dist(doc1.vectors[i], doc2.vectors[w]);
                    }
                }
            }
        }
        return acc;
    }

    double emd_relaxed(const Document &doc1, const Document &doc2)
    {
        double score = 0;
        parallel::thread_pool pool(nthreads_);
        std::vector<std::future<double>> futuress;
        futuress.reserve(nthreads_);

        size_t part = doc1.n_terms / nthreads_;
        size_t start = 0;

        std::vector<size_t > ttimes(nthreads_ + 1);
        for (size_t i = 0; i < nthreads_; i++) {
            ttimes[i] = start;
            start += part;
        }
        ttimes[nthreads_] = (int) doc1.n_terms;

        for (size_t j = 0; j < nthreads_; j++) {

            futuress.emplace_back(
                    pool.submit_task([&, j]{
                        size_t st = ttimes[j];
                        size_t en = ttimes[j + 1];
                        return emd_relaxed_thread(st, en, doc1, doc2);
                    })
            );
        }
        for (auto &fut: futuress) {
            score += fut.get();
        }
        return score;
    }

    double
    emd_relaxed_thread(const size_t start, const size_t end, const Document
    &doc1, const Document &doc2)
    {
        double acc = 0;
        std::vector<size_t > ids(doc2.n_terms);
        for (size_t i = 0; i < doc2.n_terms; ++i) {
            ids[i] = i;
        }

        auto f_c_distance = [&](size_t first, size_t second)
        {
            std::pair<size_t, size_t> pair;
            if (doc1.ids[first] < doc2.ids[second])
            {
                pair = std::make_pair(doc1.ids[first],
                                      doc2.ids[second]);
            } else {
                pair = std::make_pair(doc2.ids[second],
                                      doc1.ids[first]);
            }

            auto val = cache_->find(pair);

            if(!val)
            {
                val = dist(doc1.vectors[first],
                           doc2.vectors[second]);
                cache_->insert(pair, val.value());
            }
            return val.value();
        };

        for (size_t i = start; i < end; i++)
        {
            if (doc1.weights[i] == 0)
                continue;

            std::vector<double> distances(doc2.n_terms);

            for(size_t j = 0; j < doc2.n_terms; j++)
            {

                distances[j] = f_c_distance(i, j);
            }

            std::sort(ids.begin(),
                      ids.end(),
                      [&] (const size_t a, const size_t b) -> bool
                      {
                          return distances[a] < distances[b];
                      });

            double remaining = doc1.weights[i];
            for (auto it = ids.begin();
                 it != ids.end(); it++) {
                auto w = (uint64_t) *it;
                if (remaining < doc2.weights[w]) {
                    acc += remaining *
                           dist(doc1.vectors[i], doc2.vectors[w]);
                    break;
                } else {
                    remaining -= doc2.weights[w];
                    acc += doc2.weights[w] *
                           dist(doc1.vectors[i], doc2.vectors[w]);
                }
            }
        }
        return acc;
    }


    double wcd(Document &doc1, Document &doc2)
    {
        using namespace meta::math::operators;

        std::vector<double> res(dimension_);
        auto start = doc1.vectors.begin();
        for (auto w1: doc1.weights) {
            res = res + *start++ * w1;
        }

        start = doc2.vectors.begin();
        for (auto w2: doc2.weights) {
            res = res - *start++ * w2;
        }

        return l2norm(res);
    }

    static double
    l2diff_norm(const std::vector<double> &a, const std::vector<double> &b)
    {
        double res = 0.0;
        auto it1 = a.begin();
        auto it2 = b.begin();
        while (it1 != a.end()) {
            double val = *it1 - *it2;
            res += val * val;
            it1++;
            it2++;
        }

        return res;
    }

    static double
    cosine(const std::vector<double> &a, const std::vector<double> &b)
    {
        return -std::inner_product(a.begin(), a.end(), b.begin(), 0.0);
    }

private:

    const size_t nthreads_;
    std::shared_ptr<caching::dblru_shard_cache <std::pair<uint64_t,
            uint64_t>, double>> cache_;
    const size_t dimension_;
    const metric_type dist;
    std::unordered_map<std::string, std::function<double(const Document&,
            const Document&)>> methods_;

};


}
}

#endif //META_EMD_H
