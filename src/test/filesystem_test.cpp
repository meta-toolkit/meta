/**
 * @file filesystem_test.cpp
 * @author Chase Geigle
 */

#include <fstream>
#include "test/filesystem_test.h"
#include "io/filesystem.h"

namespace meta
{
namespace testing
{

namespace
{
void num_lines_normal()
{
    {
        std::string data = "this is a test\ntwo lines\n";
        std::ofstream file{"filesystem-temp.txt", std::ios::binary};
        file.write(data.c_str(), data.length());
    }
    ASSERT_EQUAL(filesystem::num_lines("filesystem-temp.txt"), uint64_t{2});
}

void num_lines_notrailing()
{
    {
        std::string data = "this is a test\ntwo lines but no last newline";
        std::ofstream file{"filesystem-temp.txt", std::ios::binary};
        file.write(data.c_str(), data.length());
    }
    ASSERT_EQUAL(filesystem::num_lines("filesystem-temp.txt"), uint64_t{2});
}
}

int filesystem_tests()
{
    int failed = 0;
    filesystem::delete_file("filessytem-temp.txt");
    failed += testing::run_test("num-lines-normal", num_lines_normal);
    filesystem::delete_file("filessytem-temp.txt");
    failed += testing::run_test("num-lines-notrailing", num_lines_notrailing);
    filesystem::delete_file("filessytem-temp.txt");
    return failed;
}
}
}
