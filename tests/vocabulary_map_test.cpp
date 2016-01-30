/**
 * @file vocabulary_map_test.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <iostream>

#include "bandit/bandit.h"
#include "meta/io/binary.h"
#include "meta/io/filesystem.h"
#include "meta/index/vocabulary_map_writer.h"
#include "meta/index/vocabulary_map.h"
#include "meta/util/disk_vector.h"
#include "meta/util/optional.h"

using namespace bandit;
using namespace meta;

namespace {

void write_file(uint16_t size) {
    index::vocabulary_map_writer writer{"meta-tmp-test.bin", size};
    auto str = std::string{"abcdefghijklmn"};
    for (const auto& c : str)
        writer.insert(std::string(1, c));
}

void assert_correctness(uint16_t size) {
    std::vector<std::pair<std::string, uint64_t>> expected
        = {{"a", 0},  {"b", 1},  {"c", 2},  {"d", 3}, {"e", 4},
           {"f", 5},  {"g", 6},  {"h", 7},  {"i", 8}, {"j", 9},
           {"k", 10}, {"l", 11}, {"m", 12}, {"n", 13}};

    // second level
    expected.push_back({"a", size * 0});
    expected.push_back({"c", size * 1});
    expected.push_back({"e", size * 2});
    expected.push_back({"g", size * 3});
    expected.push_back({"i", size * 4});
    expected.push_back({"k", size * 5});
    expected.push_back({"m", size * 6});

    // third level
    expected.push_back({"a", size * 7});
    expected.push_back({"e", size * 8});
    expected.push_back({"i", size * 9});
    expected.push_back({"m", size * 10});

    // root
    expected.push_back({"a", size * 11});
    expected.push_back({"i", size * 12});

    {
        std::ifstream file{"meta-tmp-test.bin", std::ios::binary};
        util::disk_vector<uint64_t> inverse{"meta-tmp-test.bin.inverse", 14};
        std::size_t idx = 0;
        while (file) {
            // skip over padding
            if (file.tellg() % size != 0 && file.peek() == '\0')
                file.seekg(size - (file.tellg() % size), file.cur);

            // checks that the inverse map contains the correct position of
            // the terms in the lowest level
            if (idx < 14)
                AssertThat(inverse[idx],
                           Equals(static_cast<uint64_t>(file.tellg())));

            std::string term;
            uint64_t num;
            io::read_binary(file, term);
            if (!file)
                break;
            io::read_binary(file, num);
            if (!file)
                break;
            AssertThat(term, Equals(expected[idx].first));
            AssertThat(num, Equals(expected[idx].second));
            ++idx;
        }
        AssertThat(file.fail(), IsTrue());
        AssertThat(idx, Equals(expected.size()));
    }

    filesystem::delete_file("meta-tmp-test.bin");
    filesystem::delete_file("meta-tmp-test.bin.inverse");
}

void read_file(uint16_t size) {
    std::vector<std::pair<std::string, uint64_t>> expected
        = {{"a", 0},  {"b", 1},  {"c", 2},  {"d", 3}, {"e", 4},
           {"f", 5},  {"g", 6},  {"h", 7},  {"i", 8}, {"j", 9},
           {"k", 10}, {"l", 11}, {"m", 12}, {"n", 13}};

    index::vocabulary_map map{"meta-tmp-test.bin", size};
    for (const auto& p : expected) {
        auto elem = map.find(p.first);
        AssertThat(static_cast<bool>(elem), IsTrue());
        AssertThat(*elem, Equals(p.second));
        AssertThat(map.find_term(term_id{p.second}), Equals(p.first));
    }
    AssertThat(static_cast<bool>(map.find("0")), IsFalse());
    AssertThat(static_cast<bool>(map.find("zabawe")), IsFalse());
    AssertThat(map.size(), Equals(14ul));
}
}

go_bandit([]() {

    describe("[vocabulary-map]", []() {

        it("should write full blocks", []() {
            write_file(20);
            assert_correctness(20);
        });

        it("should write partial blocks", []() {
            write_file(23);
            assert_correctness(23);
        });

        it("should read full blocks", []() {
            write_file(20);
            read_file(20);
        });

        it("should read partial blocks", []() {
            write_file(23);
            read_file(23);
        });
    });
});
