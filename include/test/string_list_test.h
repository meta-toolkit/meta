/**
 * @file string_list_test.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_STRING_LIST_TEST_H_
#define META_STRING_LIST_TEST_H_

#include <cstring>
#include <fstream>

#include "test/unit_test.h"
#include "io/binary.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "util/filesystem.h"

#include "unit_test.h"

namespace meta
{
namespace testing
{

void assert_read(std::ifstream& file, const std::string& expect);

struct file_guard
{
    /**
     * Always makes sure that a new file is created.
     */
    file_guard(const std::string& path) : path_{path}
    {
        filesystem::delete_file(path_);
    }

    /**
     * Always makes sure that the file is deleted when the object's destructor
     * is called.
     */
    ~file_guard()
    {
        filesystem::delete_file(path_);
    }

    const std::string& path_;
};

int string_list_tests();
}
}

#endif
