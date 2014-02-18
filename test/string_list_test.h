/**
 * @file string_list_test.h
 * @author Chase Geigle
 */

#ifndef _META_STRING_LIST_TEST_H_
#define _META_STRING_LIST_TEST_H_

#include <cstring>
#include <fstream>

#include "io/binary.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "util/filesystem.h"

#include "unit_test.h"

namespace meta
{
namespace testing
{

namespace
{
void assert_read(std::ifstream& file, const std::string& expect)
{
    std::string str;
    io::read_binary(file, str);
    ASSERT_EQUAL(str, expect);
}

struct file_guard
{
    file_guard(const std::string& path) : path_{path}
    {
        filesystem::delete_file(path_);
    }

    ~file_guard()
    {
        filesystem::delete_file(path_);
    }

    const std::string& path_;
};
}

int string_list_tests()
{
    int num_failed = 0;

    num_failed += testing::run_test("string_list_writer_basic", [&]()
    {
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

    num_failed += testing::run_test("string_list_read_basic", [&]()
    {
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
        ASSERT_EQUAL(std::strcmp(list.at(5), "wat woah this is neato"), 0);
        ASSERT_EQUAL(std::strcmp(list.at(0), "things and stuff"), 0);
        ASSERT_EQUAL(std::strcmp(list.at(2), "other stuff"), 0);
        ASSERT_EQUAL(std::strcmp(list.at(1), "cat"), 0);
        ASSERT_EQUAL(std::strcmp(list.at(4), "dog"), 0);
        ASSERT_EQUAL(std::strcmp(list.at(3), "a no good very dead ex-parrot"), 0);
    });

    testing::report(num_failed);
    return num_failed;
}
}
}

#endif
