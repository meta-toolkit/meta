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

#include "io/filesystem.h"
#include "test/probe_test.h"
#include "util/probe_map.h"
#include "util/probe_set.h"

namespace meta
{
namespace testing
{

namespace
{
template <class Set>
void count_unique(Set& set, const std::vector<std::string>& tokens)
{
    std::unordered_set<std::string> gold;
    for (const auto& token : tokens)
    {
        gold.insert(token);
        set.emplace(token);
    }

    ASSERT_EQUAL(gold.size(), set.size());

    std::vector<std::string> gold_sorted(gold.begin(), gold.end());
    std::vector<std::string> set_sorted(set.begin(), set.end());
    std::sort(gold_sorted.begin(), gold_sorted.end());
    std::sort(set_sorted.begin(), set_sorted.end());
    ASSERT(gold_sorted == set_sorted);
}

template <class Map>
void count_words(Map& map, const std::vector<std::string>& tokens)
{
    std::unordered_map<std::string, uint64_t> gold;
    for (const auto& token : tokens)
    {
        ++gold[token];
        ++map[token];
    }

    ASSERT_EQUAL(gold.size(), map.size());

    using pair_t = std::pair<std::string, uint64_t>;
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
}

int probe_tests()
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
            count_words(map, tokens);
        });

    num_failed += testing::run_test(
        "probe-map-linear-nomod", [&]()
        {
            util::probe_map<std::string, uint64_t, linear_nomod> map;
            count_words(map, tokens);
        });

    num_failed += testing::run_test(
        "probe-map-binary", [&]()
        {
            util::probe_map<std::string, uint64_t, binary> map;
            count_words(map, tokens);
        });

    num_failed += testing::run_test(
        "probe-map-quadratic", [&]()
        {
            util::probe_map<std::string, uint64_t, quadratic> map;
            // quadratic probing only works for power of two sizes
            map.resize_ratio(2.0);
            count_words(map, tokens);
        });

    return num_failed;
}
}
}
