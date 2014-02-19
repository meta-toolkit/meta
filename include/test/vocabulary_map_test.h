/**
 * @file vocabulary_map_writer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_VOCABULARY_MAP_WRITER_TEST_H_
#define _META_VOCABULARY_MAP_WRITER_TEST_H_

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

void write_file(uint16_t size = 20);

void assert_correctness(uint16_t size = 20);

void read_file(uint16_t size = 20);

int vocabulary_map_tests();
}
}

#endif
