/**
 * @file parallel_test.cpp
 * @author Sean Massung
 */

#include "test/parallel_test.h"

namespace meta
{
namespace testing
{

template <class Type>
void hard_func(Type& x)
{
    x = std::sin(x) + std::exp(std::cos(x)) / std::exp(std::sin(x));
}

template <class Type>
void easy_func(Type& x)
{
    if (x != 1.0)
        FAIL("vector contents was modified");
    --x;
}

/**
 * Assumes multi-core machine...
 */
int test_speed(std::vector<double>& v)
{
    return testing::run_test("parallel-speed", [&]()
    {

        std::iota(v.begin(), v.end(), 0);
        auto serial_time = common::time([&]()
            { std::for_each(v.begin(), v.end(), hard_func<double>); });

        std::iota(v.begin(), v.end(), 0);
        auto parallel_time = common::time([&]()
            { parallel::parallel_for(v.begin(), v.end(), hard_func<double>); });

        ASSERT_LESS(parallel_time.count(), serial_time.count());
    });
}

int test_correctness(std::vector<double>& v)
{
    // this makes sure every single element is touched exactly once
    return testing::run_test("parallel-correctness", [&]()
    {
        std::fill(v.begin(), v.end(), 1.0);
        std::mutex mtx;
        parallel::parallel_for(v.begin(), v.end(), easy_func<double>);
        ASSERT_APPROX_EQUAL(std::accumulate(v.begin(), v.end(), 0.0), 0.0);
    });
}

int test_threadpool()
{
    return testing::run_test("parallel-thread-pool", []()
    {
        parallel::thread_pool pool{};
        std::vector<std::future<size_t>> futures;

        for (size_t i = 0; i < 16; ++i)
            futures.emplace_back(pool.submit_task([]() { return size_t{1}; }));

        size_t sum = 0;
        for (auto& fut : futures)
        {
            auto val = fut.get();
            ASSERT_EQUAL(val, size_t{1});
            sum += val;
        }

        ASSERT_EQUAL(sum, size_t{16});
    });
}

int parallel_tests()
{
    size_t n = 10000000;
    std::vector<double> v(n);
    int num_failed = 0;
    num_failed += test_speed(v);
    num_failed += test_correctness(v);
    num_failed += test_threadpool();
    return num_failed;
}
}
}
