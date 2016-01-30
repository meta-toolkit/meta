/**
 * @file string_list_test.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <cstring>
#include <fstream>

#include "bandit/bandit.h"
#include "meta/index/string_list.h"
#include "meta/index/string_list_writer.h"
#include "meta/io/binary.h"
#include "meta/io/filesystem.h"

using namespace bandit;
using namespace meta;

namespace {

void assert_read(std::ifstream& file, const std::string& expected) {
    std::string str;
    io::read_binary(file, str);
    AssertThat(str, Equals(expected));
}

/**
 * Always makes sure a new file is created.
 */
struct file_guard {
    /**
     * Always makes sure that a new file is created.
     */
    file_guard(const std::string& path) : path_{path} {
        filesystem::delete_file(path_);
    }

    /**
     * Always makes sure that the file is deleted when the object's destructor
     * is called.
     */
    ~file_guard() {
        filesystem::delete_file(path_);
    }
    /// The path to this file
    std::string path_;
};
}

go_bandit([]() {

    describe("[string-list]", []() {

        it("should write strings", [&]() {
            file_guard f{"meta-tmp-string-list.bin"};
            file_guard fi{"meta-tmp-string-list.bin_index"};
            using namespace index;
            {
                string_list_writer writer{"meta-tmp-string-list.bin", 5};
                writer.insert(5, "wat woah this is neato");
                writer.insert(0, "things and stuff");
                writer.insert(2, "other stuff");
                writer.insert(1, "cat");
                writer.insert(4, "dog");
                writer.insert(3, "a no good very dead ex-parrot");
            }
            std::ifstream file{"meta-tmp-string-list.bin", std::ios::binary};
            assert_read(file, "wat woah this is neato");
            assert_read(file, "things and stuff");
            assert_read(file, "other stuff");
            assert_read(file, "cat");
            assert_read(file, "dog");
            assert_read(file, "a no good very dead ex-parrot");
        });

        it("should read strings", [&]() {
            file_guard f{"meta-tmp-string-list.bin"};
            file_guard fi{"meta-tmp-string-list.bin_index"};
            using namespace index;
            {
                string_list_writer writer{"meta-tmp-string-list.bin", 5};
                writer.insert(5, "wat woah this is neato");
                writer.insert(0, "things and stuff");
                writer.insert(2, "other stuff");
                writer.insert(1, "cat");
                writer.insert(4, "dog");
                writer.insert(3, "a no good very dead ex-parrot");
            }

            string_list list{"meta-tmp-string-list.bin"};
            AssertThat(list.at(5), Equals("wat woah this is neato"));
            AssertThat(list.at(0), Equals("things and stuff"));
            AssertThat(list.at(2), Equals("other stuff"));
            AssertThat(list.at(1), Equals("cat"));
            AssertThat(list.at(4), Equals("dog"));
            AssertThat(list.at(3), Equals("a no good very dead ex-parrot"));
        });
    });
});
