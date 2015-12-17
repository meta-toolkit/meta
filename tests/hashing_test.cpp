/**
 * @file hashing_test.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "bandit/bandit.h"
#include "hashing/probe_map.h"
#include "hashing/probe_set.h"
#include "io/filesystem.h"

using namespace bandit;
using namespace meta;

namespace {

/**
 * Checks that a probing strategy probes each element in a range exactly once.
 */
template <class Strategy>
void check_range_at(uint64_t hash, uint64_t size) {
    std::vector<uint64_t> checker(size, 0);
    const std::vector<uint64_t> gold(size, 1);
    Strategy strat{hash, size};
    for (uint64_t i = 0; i < checker.size(); ++i)
        ++checker[strat.probe()];

    AssertThat(checker, Equals(gold));
}

template <class Strategy>
void check_range() {
    std::vector<uint64_t> sizes = {2, 4, 8, 32, 64};
    std::vector<uint64_t> weird_sizes = {3, 5, 7, 22, 100, 125};
    if (!std::is_same<Strategy, hashing::probing::quadratic>::value)
        sizes.insert(sizes.end(), weird_sizes.begin(), weird_sizes.end());

    for (const auto& size : sizes) {
        check_range_at<Strategy>(0, size);
        check_range_at<Strategy>(1, size);
        check_range_at<Strategy>(2, size);
        check_range_at<Strategy>(3, size);
        check_range_at<Strategy>(19, size);
        check_range_at<Strategy>(64, size);
        check_range_at<Strategy>(34985764, size);
        check_range_at<Strategy>(20857211, size);
    }
}

template <class Set, class T>
void count_unique(Set& set, const std::vector<T>& tokens) {
    std::unordered_set<T> gold;
    for (const auto& token : tokens) {
        gold.insert(token);
        set.emplace(token);
    }

    AssertThat(gold.size(), Equals(set.size()));

    std::vector<T> gold_sorted(gold.begin(), gold.end());
    std::vector<T> set_sorted(set.begin(), set.end());
    std::sort(gold_sorted.begin(), gold_sorted.end());
    std::sort(set_sorted.begin(), set_sorted.end());
    AssertThat(gold_sorted, Equals(set_sorted));
}

// exercise the const versions of functions
template <class Map, class K>
void compare(const Map& map, const std::unordered_map<K, uint64_t>& gold) {
    using value_type = typename std::unordered_map<K, uint64_t>::value_type;
    for (const auto& pr : gold) {
        auto it = map.find(pr.first);
        AssertThat(it->value(), Equals(pr.second));
        AssertThat(static_cast<value_type>(*it), Equals(pr));
    }
    AssertThat(map.size(), Equals(gold.size()));
}

template <class Map, class K>
void count(Map& map, const std::vector<K>& tokens) {
    std::unordered_map<K, uint64_t> gold;
    for (const auto& token : tokens) {
        ++gold[token];
        ++map[token];
    }

    AssertThat(gold.size(), Equals(map.size()));

    using pair_t = std::pair<K, uint64_t>;
    auto comp
        = [](const pair_t& a, const pair_t& b) { return a.first < b.first; };

    std::vector<pair_t> gold_sorted(gold.begin(), gold.end());
    std::vector<pair_t> map_sorted(map.begin(), map.end());
    std::sort(gold_sorted.begin(), gold_sorted.end(), comp);
    std::sort(map_sorted.begin(), map_sorted.end(), comp);
    AssertThat(gold_sorted, Equals(map_sorted));

    compare(map, gold);
}
}

go_bandit([]() {

    describe("[hashing] ints", []() {

        using namespace hashing::probing;

        std::string in_path{"../data/ceeaus-metadata.txt"};
        std::string input = filesystem::file_text(in_path);
        std::vector<uint64_t> numbers;
        std::istringstream iss{input};
        std::copy(std::istream_iterator<uint64_t>(iss),
                  std::istream_iterator<uint64_t>(),
                  std::back_inserter(numbers));

        it("should use linear probing (probe_set)", [&]() {
            hashing::probe_set<uint64_t, linear> set;
            count_unique(set, numbers);
        });

        it("should use linear probing without mod (probe_set)", [&]() {
            hashing::probe_set<uint64_t, linear_nomod> set;
            count_unique(set, numbers);
        });

        it("should use binary probing (probe_set)", [&]() {
            hashing::probe_set<uint64_t, binary> set;
            count_unique(set, numbers);
        });

        it("should use binary hybrid probing (probe_set)", [&]() {
            hashing::probe_set<uint64_t, binary_hybrid<uint64_t>> set;
            count_unique(set, numbers);
        });

        it("should use quadratic probing (probe_set)", [&]() {
            hashing::probe_set<uint64_t, quadratic> set;
            // quadratic probing only works for power-of-two sizes
            set.resize_ratio(2.0);
            count_unique(set, numbers);
        });

        it("should use linear probing (probe_map)", [&]() {
            hashing::probe_map<uint64_t, uint64_t, linear> map;
            count(map, numbers);
        });

        it("should use linear probing without mod (probe_map)", [&]() {
            hashing::probe_map<uint64_t, uint64_t, linear_nomod> map;
            count(map, numbers);
        });

        it("should use binary probing (probe_map)", [&]() {
            hashing::probe_map<uint64_t, uint64_t, binary> map;
            count(map, numbers);
        });

        it("should use binary hybrid probing (probe_map)", [&]() {
            using stored_type = std::pair<uint64_t, uint64_t>;
            using probing_strat = hashing::probing::binary_hybrid<stored_type>;
            hashing::probe_map<uint64_t, uint64_t, probing_strat> map;
            count(map, numbers);
        });

        it("should use quadratic probing (probe_map)", [&]() {
            hashing::probe_map<uint64_t, uint64_t, quadratic> map;
            // quadratic probing only works for power of two sizes
            map.resize_ratio(2.0);
            count(map, numbers);
        });
    });

    describe("[hashing] strings", []() {
        using namespace hashing::probing;

        std::string in_path{"../data/sample-document.txt"};
        std::string input = filesystem::file_text(in_path);
        std::vector<std::string> tokens;
        std::istringstream iss{input};
        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(tokens));

        it("should use linear probing (probe_set)", [&]() {
            hashing::probe_set<std::string, linear> set;
            count_unique(set, tokens);
        });

        it("should use linear probing without mod (probe_set)", [&]() {
            hashing::probe_set<std::string, linear_nomod> set;
            count_unique(set, tokens);
        });

        it("should use binary probing (probe_set)", [&]() {
            hashing::probe_set<std::string, binary> set;
            count_unique(set, tokens);
        });

        it("should use binary hybrid probing (probe_set)", [&]() {
            hashing::probe_set<std::string, binary_hybrid<std::size_t>> set;
            count_unique(set, tokens);
        });

        it("should use quadratic probing (probe_set)", [&]() {
            hashing::probe_set<std::string, quadratic> set;
            // quadratic probing only works for power-of-two sizes
            set.resize_ratio(2.0);
            count_unique(set, tokens);
        });

        it("should use linear probing (probe_map)", [&]() {
            hashing::probe_map<std::string, uint64_t, linear> map;
            count(map, tokens);
        });

        it("should use linear probing without mod (probe_map)", [&]() {
            hashing::probe_map<std::string, uint64_t, linear_nomod> map;
            count(map, tokens);
        });

        it("should use binary probing (probe_map)", [&]() {
            hashing::probe_map<std::string, uint64_t, binary> map;
            count(map, tokens);
        });

        it("should use binary hybrid probing (probe_map)", [&]() {
            using probe_strat = binary_hybrid<std::size_t>;
            hashing::probe_map<std::string, uint64_t, probe_strat> map;
            count(map, tokens);
        });

        it("should use quadratic probing (probe_map)", [&]() {
            hashing::probe_map<std::string, uint64_t, quadratic> map;
            // quadratic probing only works for power of two sizes
            map.resize_ratio(2.0);
            count(map, tokens);
        });
    });

    describe("[hashing] probing", []() {

        it("should visit all slots in the table (linear)",
           []() { check_range<hashing::probing::linear>(); });

        it("should visit all slots in the table (linear_nomod)",
           []() { check_range<hashing::probing::linear_nomod>(); });

        it("should visit all slots in the table (binary)",
           []() { check_range<hashing::probing::binary>(); });

        it("should visit all slots in the table (binary_hybrid)",
           []() { check_range<hashing::probing::binary_hybrid<uint64_t>>(); });

        it("should visit all slots in the table (quadratic)",
           []() { check_range<hashing::probing::quadratic>(); });

    });
});
