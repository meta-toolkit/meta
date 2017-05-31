/**
 * @file dataset_transform_test.cpp
 * @author Chase Geigle
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/classify/multiclass_dataset.h"
#include "meta/index/ranker/okapi_bm25.h"
#include "meta/learn/transform.h"

using namespace bandit;
using namespace meta;

go_bandit([]() {
    describe("[learn] dataset l2 transformer", []() {
        it("should normalize feature vectors to unit length", []() {

            std::vector<learn::feature_vector> vectors(2);

            vectors[0].emplace_back(0_tid, 12);
            vectors[0].emplace_back(1_tid, 10);
            vectors[0].emplace_back(2_tid, 5);

            vectors[1].emplace_back(1_tid, 1);
            vectors[1].emplace_back(3_tid, 4);
            vectors[1].emplace_back(5_tid, 9);

            learn::dataset dset{vectors.begin(), vectors.end(), 6};
            learn::l2norm_transform(dset);

            for (const auto& inst : dset)
            {
                auto norm = std::sqrt(std::accumulate(
                    inst.weights.begin(), inst.weights.end(), 0.0,
                    [](double accum, const std::pair<term_id, double>& val) {
                        return accum + val.second * val.second;
                    }));
                AssertThat(norm, EqualsWithDelta(1, 1e-12));
            }
        });
    });

    describe("[learn] dataset tf-idf transformer", []() {
        it("should produce tf-idf vectors", []() {
            auto config = tests::create_config("line");
            config->insert("uninvert", true);
            filesystem::remove_all("ceeaus");

            // make both indexes
            auto inv = index::make_index<index::inverted_index>(*config);
            auto fwd = index::make_index<index::forward_index>(*config);

            // convert the data into a dataset
            classify::multiclass_dataset dset{fwd};

            // make tf-idf vectors
            index::okapi_bm25 ranker;
            learn::tfidf_transform(dset, *inv, ranker);

            // check that we get the same scores for a particular word
            std::vector<std::pair<std::string, double>> query
                = {{"charact", 1.0}};

            auto ranking = ranker.score(*inv, query.begin(), query.end());

            auto tid = inv->get_term_id("charact");
            for (const auto& result : ranking)
            {
                const auto& weights = dset(result.d_id).weights;
                AssertThat(weights.at(tid),
                           EqualsWithDelta(result.score, 1e-5));
            }
        });
    });
});
