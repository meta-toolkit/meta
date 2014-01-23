/**
 * @file unit-test.cpp
 */

#include "unit_test.h"
//#include "tokenizer_test.h"
#include "index_test.h"
#include "vocabulary_map_writer_test.h"

using namespace meta;

int main(int argc, char* argv[])
{
    //testing::tokenizer_tests();
    testing::index_tests();
    testing::vocabulary_map_writer_tests();
}
