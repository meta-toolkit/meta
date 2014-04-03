/**
 * @file compression_test.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <numeric>
#include <random>
#include "util/filesystem.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "test/compression_test.h"

namespace meta
{
namespace testing
{

int compression_tests()
{
    int num_failed = 0;

    std::string filename{"meta-tmp-compressed.dat"};
    std::string str{"some random string"};
    std::vector<uint64_t> vec(100, 0);
    std::iota(vec.begin(), vec.end(), 1);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vec.begin(), vec.end(), g);

    num_failed += testing::run_test("compressed-file-writer", [&]()
    {
        io::compressed_file_writer writer{filename,
                                          io::default_compression_writer_func};
        writer.write(str);
        for (auto& v : vec)
            writer.write(v);
        writer.write(str);
    });

    num_failed += testing::run_test("compressed-file-reader", [&]()
    {
        io::compressed_file_reader reader{filename,
                                          io::default_compression_reader_func};
        ASSERT_EQUAL(reader.next_string(), "some random string");
        for (auto& v : vec)
            ASSERT_EQUAL(reader.next(), v);
        ASSERT_EQUAL(reader.next_string(), "some random string");
    });

    if (filesystem::file_exists(filename))
        filesystem::delete_file(filename);

    return num_failed;
}
}
}
