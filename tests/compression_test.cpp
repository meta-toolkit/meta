/**
 * @file compression_test.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <numeric>
#include <random>

#include "bandit/bandit.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "io/filesystem.h"

using namespace bandit;
using namespace meta;

go_bandit([]() {

    describe("[compression]", []() {

        std::string filename{"meta-tmp-compressed.dat"};
        std::string str{"some random string"};
        std::vector<uint64_t> vec(100, 0);
        std::iota(vec.begin(), vec.end(), 1);
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(vec.begin(), vec.end(), g);

        it("should write compressed files", [&]() {
            io::compressed_file_writer writer{
                filename, io::default_compression_writer_func};
            writer.write(str);
            for (auto& v : vec)
                writer.write(v);
            writer.write(str);
        });

        it("should read compressed files", [&]() {
            io::compressed_file_reader reader{
                filename, io::default_compression_reader_func};
            AssertThat(reader.next_string(), Equals("some random string"));
            for (auto& v : vec)
                AssertThat(reader.next(), Equals(v));
            AssertThat(reader.next_string(), Equals("some random string"));
        });

        it("should be able to have files deleted",
           [&]() { AssertThat(filesystem::delete_file(filename), IsTrue()); });
    });
});
