/**
 * @file unit-test.cpp
 */

#include "unit_test.h"
#include "tokenizer_test.h"
#include "inverted_index_test.h"
#include "string_list_test.h"
#include "vocabulary_map_test.h"
#include "libsvm_parser_test.h"

using namespace meta;

int main(int argc, char* argv[])
{
    testing::tokenizer_tests();
    testing::index_tests();
    testing::string_list_tests();
    testing::vocabulary_map_tests();
    testing::libsvm_parser_tests();
}
