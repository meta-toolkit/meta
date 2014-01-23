/**
 * @file vocabulary_map_writer.h
 * @author Chase Geigle
 */

#ifndef _META_VOCABULARY_MAP_WRITER_TEST_H_
#define _META_VOCABULARY_MAP_WRITER_TEST_H_

#include <iostream>
#include "util/disk_vector.h"
#include "util/filesystem.h"
#include "unit_test.h"
#include "index/vocabulary_map_writer.h"
#include "index/vocabulary_map.h"

namespace meta
{
namespace testing
{

namespace
{
void write_file(uint16_t size = 20)
{
    index::vocabulary_map_writer writer{"meta-tmp-test.bin", size};
    auto str = std::string{"abcdefghijklmn"};
    for (const auto& c : str)
        writer.insert(std::string(1, c));
}

void assert_correctness(uint16_t size = 20)
{
    std::vector<std::pair<std::string, uint64_t>> expected = {
        // first level
        {"a", 0},
        {"b", 1},
        {"c", 2},
        {"d", 3},
        {"e", 4},
        {"f", 5},
        {"g", 6},
        {"h", 7},
        {"i", 8},
        {"j", 9},
        {"k", 10},
        {"l", 11},
        {"m", 12},
        {"n", 13}};

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
        size_t idx = 0;
        while (file)
        {
            // skip over padding
            if (file.tellg() % size != 0 && file.peek() == '\0')
                file.seekg(size - (file.tellg() % size), file.cur);

            // checks that the inverse map contains the correct position of
            // the terms in the lowest level
            if (idx < 14)
                ASSERT(inverse[idx] == file.tellg());

            std::string term;
            uint64_t num;
            common::read_binary(file, term);
            if (!file)
                break;
            common::read_binary(file, num);
            if (!file)
                break;
            ASSERT(term == expected[idx].first);
            ASSERT(num == expected[idx].second);
            ++idx;
        }
        ASSERT(!file);
        ASSERT(idx == expected.size());
    }

    filesystem::delete_file("meta-tmp-test.bin");
    filesystem::delete_file("meta-tmp-test.bin.inverse.vector");
}

void read_file(uint16_t size = 20)
{
    std::vector<std::pair<std::string, uint64_t>> expected = {
        // first level
        {"a", 0},
        {"b", 1},
        {"c", 2},
        {"d", 3},
        {"e", 4},
        {"f", 5},
        {"g", 6},
        {"h", 7},
        {"i", 8},
        {"j", 9},
        {"k", 10},
        {"l", 11},
        {"m", 12},
        {"n", 13}};

    index::vocabulary_map map{"meta-tmp-test.bin", size};
    for (const auto& p : expected)
    {
        auto elem = map.find(p.first);
        ASSERT(elem);
        ASSERT(*elem == p.second);
    }
    ASSERT(!map.find("0"));
    ASSERT(!map.find("zabawe"));
}
}

void vocabulary_map_tests()
{
    testing::run_test("vocabulary_writer_full_block", [&]()
    {
        write_file();
        assert_correctness();
    });

    testing::run_test("vocabulary_writer_partial_blocks", [&]()
    {
        write_file(23);
        assert_correctness(23);
    });

    testing::run_test("vocabulary_map_full_block", [&]()
    {
        write_file();
        read_file();
    });

    testing::run_test("vocabulary_map_partial_blocks", [&]()
    {
        write_file(23);
        read_file(23);
    });
}
}
}

#endif
