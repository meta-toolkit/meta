/**
 * @file unit-test.cpp
 */

#include <iostream>
#include "test/unit_test.h"

using namespace meta;

int main(int argc, char* argv[])
{
    testing::run_test("assert false", []() {
        ASSERT(2 == 1);
    });

    testing::run_test("assert true", []() {
        ASSERT(1 == 1);
    });

    testing::run_test("test fail", []() {
        FAIL("Because I say so");
    });

    testing::run_test("test empty", [](){});

    testing::run_test("test sleep fail", []() {
        sleep(2);
    });

    testing::run_test("test sleep pass", 3, []() {
        sleep(2);
    });

    testing::run_test("fail no line", [](){
        FAIL_NOLINE("You failed, no line number given");
    });
}
