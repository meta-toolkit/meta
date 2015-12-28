/**
 * @file parallel_test.cpp
 * @author Sean Massung
 */

#include <cmath>
#include <algorithm>
#include <numeric>

#include "bandit/bandit.h"
#include "meta/util/time.h"
#include "meta/parallel/parallel_for.h"
#include "meta/parallel/thread_pool.h"

using namespace bandit;
using namespace meta;

namespace {

void hard_func(double& x) {
    x = std::sin(x) + std::exp(std::cos(x)) / std::exp(std::sin(x));
}

void easy_func(double& x) {
    AssertThat(x, Equals(1.0));
    --x;
}
}

go_bandit([]() {

    std::vector<double> v(10000000);
    describe("[parallel] parallel_for", [&]() {

        it("should produce correct calculations in parallel", [&]() {
            std::fill(v.begin(), v.end(), 1.0);
            std::mutex mtx;
            parallel::parallel_for(v.begin(), v.end(), easy_func);
            AssertThat(std::accumulate(v.begin(), v.end(), 0.0),
                       EqualsWithDelta(0.0, 0.0000001));
        });

        it("should be faster in parallel", [&]() {

            // if we only get one thread on this machine, skip the speed test
            if (std::thread::hardware_concurrency() > 1) {

                std::iota(v.begin(), v.end(), 0);
                auto serial_time = common::time(
                    [&]() { std::for_each(v.begin(), v.end(), hard_func); });

                std::iota(v.begin(), v.end(), 0);
                auto parallel_time = common::time([&]() {
                    parallel::parallel_for(v.begin(), v.end(), hard_func);
                });

                AssertThat(parallel_time.count(),
                           Is().LessThan(serial_time.count()));
            }
        });
    });

    describe("[parallel] thread pool", []() {

        parallel::thread_pool pool{};
        std::vector<std::future<size_t>> futures;

        it("should create the correct number of threads", [&]() {

            for (std::size_t i = 0; i < 16; ++i)
                futures.emplace_back(
                    pool.submit_task([]() { return std::size_t{1}; }));

            std::size_t sum = 0;
            for (auto& fut : futures) {
                auto val = fut.get();
                AssertThat(val, Equals(std::size_t{1}));
                sum += val;
            }

            AssertThat(sum, Equals(std::size_t{16}));
        });
    });
});
