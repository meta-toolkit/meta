/**
 * @file unit_test.h
 *
 * Contains macros used in unit testing.
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <string>
#include <iostream>
#include "util/common.h"

#define ASSERT(expr) \
    do { \
       if (!(expr)) \
           FAIL(Assertion failed: #expr ); \
    } while(0)

#define FAIL(why) \
    do { \
        std::cerr << Common::makeRed("FAIL") << " " << #why << " ("; \
        std::cerr << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        return; \
    } while(0)

#define PASS \
    do { \
        std::cerr << Common::makeGreen("OK") << std::endl; \
        return; \
    } while(0)

/**
 *
 */
namespace UnitTests
{
    using std::string;
    using std::endl;
    using std::cerr;

    /**
     *
     */
    template <class Func>
    void runTest(const string & testName, Func func)
    {
        cerr << " " << testName << "... ";
        func();
    }
}

#endif
