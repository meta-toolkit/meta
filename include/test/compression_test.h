/**
 * @file compression_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_COMPRESSION_TEST_H_
#define META_COMPRESSION_TEST_H_

#include "test/unit_test.h"

namespace meta
{
namespace testing
{

/**
 * Tests compressed_file_reader and compressed_file_writer.
 */
int compression_tests();
}
}

#endif
