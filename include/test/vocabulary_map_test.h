/**
 * @file vocabulary_map_writer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_VOCABULARY_MAP_WRITER_TEST_H_
#define META_VOCABULARY_MAP_WRITER_TEST_H_

#include <iostream>
#include "io/binary.h"
#include "index/vocabulary_map_writer.h"
#include "index/vocabulary_map.h"
#include "util/disk_vector.h"
#include "util/filesystem.h"
#include "test/unit_test.h"

namespace meta
{
namespace testing
{
/**
 * Writes a file to decode
 * @param size The number of bytes in the file
 */
void write_file(uint16_t size = 20);

/**
 * Makes sure the content in the vocab map is correct.
 * @param size The number of bytes in the file
 */
void assert_correctness(uint16_t size = 20);

/**
 * Reads data from the vocab map file.
 * @param size The number of bytes in the file
 */
void read_file(uint16_t size = 20);

/**
 * Runs the vocab map tests.
 * @return the number of tests failed
 */
int vocabulary_map_tests();
}
}

#endif
