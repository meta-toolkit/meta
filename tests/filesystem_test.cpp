/**
 * @file filesystem_test.cpp
 * @author Chase Geigle
 * @author Sean Massung
 */

#include <fstream>

#include "bandit/bandit.h"
#include "meta/io/filesystem.h"

using namespace bandit;
using namespace meta;

go_bandit([]() {

    describe("[filesystem] num_lines", []() {
        const std::string filename{"filesystem-temp.txt"};

        it("should count the number of lines", [&]() {
            {
                std::string data = "this is a test\ntwo lines\n";
                std::ofstream file{filename, std::ios::binary};
                file.write(data.c_str(),
                           static_cast<std::streamsize>(data.length()));
            }
            AssertThat(filesystem::num_lines(filename), Equals(uint64_t{2}));
        });

        filesystem::delete_file(filename);

        it("should count the number of lines with no trailing newline", [&]() {
            {
                std::string data = "this is a test\nwith no last newline";
                std::ofstream file{filename, std::ios::binary};
                file.write(data.c_str(),
                           static_cast<std::streamsize>(data.length()));
            }
            AssertThat(filesystem::num_lines(filename), Equals(uint64_t{2}));
        });

        filesystem::delete_file(filename);
    });
});
