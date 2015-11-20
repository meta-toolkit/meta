/**
 * @file probe_test.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iterator>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "hashing/probe_map.h"
#include "hashing/probe_set.h"
#include "io/filesystem.h"
#include "test/probe_test.h"

namespace meta
{
namespace testing
{

namespace
{
template <class Set, class T>
void count_unique(Set& set, const std::vector<T>& tokens)
{
    std::unordered_set<T> gold;
    for (const auto& token : tokens)
    {
        gold.insert(token);
        set.emplace(token);
    }

    ASSERT_EQUAL(gold.size(), set.size());

    std::vector<T> gold_sorted(gold.begin(), gold.end());
    std::vector<T> set_sorted(set.begin(), set.end());
    std::sort(gold_sorted.begin(), gold_sorted.end());
    std::sort(set_sorted.begin(), set_sorted.end());
    ASSERT(gold_sorted == set_sorted);
}

template <class Map, class K>
void count(Map& map, const std::vector<K>& tokens)
{
    std::unordered_map<K, uint64_t> gold;
    for (const auto& token : tokens)
    {
        ++gold[token];
        ++map[token];
    }

    ASSERT_EQUAL(gold.size(), map.size());

    using pair_t = std::pair<K, uint64_t>;
    auto comp = [](const pair_t& a, const pair_t& b)
    {
        return a.first < b.first;
    };

    std::vector<pair_t> gold_sorted(gold.begin(), gold.end());
    std::vector<pair_t> map_sorted(map.begin(), map.end());
    std::sort(gold_sorted.begin(), gold_sorted.end(), comp);
    std::sort(map_sorted.begin(), map_sorted.end(), comp);
    ASSERT(gold_sorted == map_sorted);
}

int string_tests()
{
    using namespace util::probing;

    int num_failed = 0;

    std::string in_path{"../data/sample-document.txt"};
    std::string input = filesystem::file_text(in_path);
    std::vector<std::string> tokens;
    std::istringstream iss{input};
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(), std::back_inserter(tokens));

    num_failed
        += testing::run_test("probe-set-linear", [&]()
                             {
                                 util::probe_set<std::string, linear> set;
                                 count_unique(set, tokens);
                             });

    num_failed
        += testing::run_test("probe-set-linear-nomod", [&]()
                             {
                                 util::probe_set<std::string, linear_nomod> set;
                                 count_unique(set, tokens);
                             });

    num_failed
        += testing::run_test("probe-set-binary", [&]()
                             {
                                 util::probe_set<std::string, binary> set;
                                 count_unique(set, tokens);
                             });

    num_failed
        += testing::run_test("probe-set-quadratic", [&]()
                             {
                                 util::probe_set<std::string, quadratic> set;
                                 // quadratic probing only works for power of
                                 // two sizes
                                 set.resize_ratio(2.0);
                                 count_unique(set, tokens);
                             });

    num_failed += testing::run_test(
        "probe-map-linear", [&]()
        {
            util::probe_map<std::string, uint64_t, linear> map;
            count(map, tokens);
        });

    num_failed += testing::run_test(
        "probe-map-linear-nomod", [&]()
        {
            util::probe_map<std::string, uint64_t, linear_nomod> map;
            count(map, tokens);
        });

    num_failed += testing::run_test(
        "probe-map-binary", [&]()
        {
            util::probe_map<std::string, uint64_t, binary> map;
            count(map, tokens);
        });

    num_failed += testing::run_test(
        "probe-map-quadratic", [&]()
        {
            util::probe_map<std::string, uint64_t, quadratic> map;
            // quadratic probing only works for power of two sizes
            map.resize_ratio(2.0);
            count(map, tokens);
        });

    return num_failed;
}

int int_tests()
{
    using namespace util::probing;

    int num_failed = 0;

    std::string in_path{"../data/ceeaus-metadata.txt"};
    std::string input = filesystem::file_text(in_path);
    std::vector<uint64_t> numbers;
    std::istringstream iss{input};
    std::copy(std::istream_iterator<uint64_t>(iss),
              std::istream_iterator<uint64_t>(), std::back_inserter(numbers));

    num_failed += testing::run_test("probe-set-linear-inline", [&]()
                                    {
                                        util::probe_set<uint64_t, linear> set;
                                        count_unique(set, numbers);
                                    });

    num_failed
        += testing::run_test("probe-set-linear-nomod-inline", [&]()
                             {
                                 util::probe_set<uint64_t, linear_nomod> set;
                                 count_unique(set, numbers);
                             });

    num_failed += testing::run_test("probe-set-binary-inline", [&]()
                                    {
                                        util::probe_set<uint64_t, binary> set;
                                        count_unique(set, numbers);
                                    });

    num_failed
        += testing::run_test("probe-set-quadratic-inline", [&]()
                             {
                                 util::probe_set<uint64_t, quadratic> set;
                                 // quadratic probing only works for power of
                                 // two sizes
                                 set.resize_ratio(2.0);
                                 count_unique(set, numbers);
                             });

    num_failed += testing::run_test(
        "probe-map-linear-inline", [&]()
        {
            util::probe_map<uint64_t, uint64_t, linear> map;
            count(map, numbers);
        });

    num_failed += testing::run_test(
        "probe-map-linear-nomod-inline", [&]()
        {
            util::probe_map<uint64_t, uint64_t, linear_nomod> map;
            count(map, numbers);
        });

    num_failed += testing::run_test(
        "probe-map-binary-inline", [&]()
        {
            util::probe_map<uint64_t, uint64_t, binary> map;
            count(map, numbers);
        });

    num_failed += testing::run_test(
        "probe-map-quadratic-inline", [&]()
        {
            util::probe_map<uint64_t, uint64_t, quadratic> map;
            // quadratic probing only works for power of two sizes
            map.resize_ratio(2.0);
            count(map, numbers);
        });

    return num_failed;
}
}

int probe_tests()
{
    int num_failed = 0;
    num_failed += string_tests();
    num_failed += int_tests();
    return num_failed;
}
}
}
