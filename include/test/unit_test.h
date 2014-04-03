/**
 * @file unit_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UNIT_TEST_H_
#define META_UNIT_TEST_H_

#include <cmath>
#include <iomanip>
#include <string>
#include <functional>
#include <iostream>
#include "util/printing.h"

/**
 * Fail if expr is false; otherwise continue.
 */
#define ASSERT(expr)                                                           \
    do                                                                         \
    {                                                                          \
        if (!(expr))                                                           \
            FAIL("Assertion failed: " #expr);                                  \
    } while (0)

/**
 * Fail if exp1 != exp2; otherwise continue.
 * @see https://bitbucket.org/jacktoole1/monad_hg/ for string printing
 */
#define ASSERT_EQUAL(exp1, exp2)                                               \
    do                                                                         \
    {                                                                          \
        std::string msg = testing::assert_equal(exp1, exp2, #exp1, #exp2);     \
        if (!msg.empty())                                                      \
            FAIL(msg);                                                         \
    } while (0)

/**
 * Fail if !(|exp1 - exp2| < epsilon); otherwise continue.
 */
#define ASSERT_APPROX_EQUAL(exp1, exp2)                                        \
    do                                                                         \
    {                                                                          \
        std::string msg                                                        \
            = testing::assert_approx_equal(exp1, exp2, #exp1, #exp2);          \
        if (!msg.empty())                                                      \
            FAIL(msg);                                                         \
    } while (0)

/**
 * Fail if !binop(exp1, exp2).
 */
#define ASSERT_BINOP(exp1, exp2, binop)                                        \
    do                                                                         \
    {                                                                          \
        std::string msg = testing::assert(exp1, exp2, #exp1, #exp2, binop);    \
        if (!msg.empty())                                                      \
            FAIL(msg);                                                         \
    } while (0)

/**
 * Fail this test case with an explanation.
 * Give line number and file where the test case failed.
 */
#define FAIL(why)                                                              \
    do                                                                         \
    {                                                                          \
        std::string fail_msg = "[ " + printing::make_red("FAIL") + " ] "       \
                               + (why) + " (" + testing::filename(__FILE__)    \
                               + ":" + std::to_string(__LINE__) + ")";         \
        throw testing::unit_test_exception{fail_msg};                          \
    } while (0)

/**
 * Fail if !(exp1 < exp2)
 */
#define ASSERT_LESS(exp1, exp2)                                                \
    do                                                                         \
    {                                                                          \
        std::string msg = testing::assert_less(exp1, exp2, #exp1, #exp2);      \
        if (!msg.empty())                                                      \
            FAIL(msg);                                                         \
    } while (0)

/**
 * Fail if !(exp1 > exp2)
 */
#define ASSERT_GREATER(exp1, exp2)                                             \
    do                                                                         \
    {                                                                          \
        std::string msg = testing::assert_greater(exp1, exp2, #exp1, #exp2);   \
        if (!msg.empty())                                                      \
            FAIL(msg);                                                         \
    } while (0)

namespace meta
{

/**
 * Contains unit testing functions for the META toolkit.
 */
namespace testing
{
/// Used to compare floating point equality
static double epsilon = 0.0000001;

/**
 * @param path The path to truncate
 */
inline std::string filename(const std::string& path)
{
    size_t slash = path.find_last_of("/\\");
    if(slash == std::string::npos)
        return path;
    return path.substr(slash + 1);
}

/**
 * Allows the user to see what the evaluated statements are.
 * @param expected The expected expression
 * @param actual The actual expression
 * @param expstr The expected string
 * @param actstr The actual string
 * @param binop The binary operator to compare the expressions; by default
 * std::equal_to
 */
template <class T, class K, class BinOp>
inline std::string assert_equal(const T& expected, const K& actual,
                                const char* expstr, const char* actstr,
                                BinOp&& binop)
{
    if (!binop(expected, actual))
    {
        std::stringstream ss;
        ss << "[" << expstr << " == " << actstr << "] => [" << expected
           << " == " << actual << "]";
        return ss.str();
    }
    return "";
}

/**
 * @param expected The expected expression
 * @param actual The actual expression
 * @param expstr The expected string
 * @param actstr The actual string
 */
template <class T, class K>
inline std::string assert_equal(const T& expected, const K& actual,
                                const char* expstr, const char* actstr)
{
    return assert_equal(expected, actual, expstr, actstr,
            [](const T& a, const K& b)
    {
        return a == b;
    });
}

/**
 * @param expected The expected expression
 * @param actual The actual expression
 * @param expstr The expected string
 * @param actstr The actual string
 */
template <class T, class K>
inline std::string assert_approx_equal(const T& expected, const K& actual,
                                       const char* expstr, const char* actstr)
{
    if (!(std::abs(expected - actual) < epsilon))
    {
        std::stringstream ss;
        ss << "[abs(" << expstr << " - " << actstr << ") < epsilon] => [abs("
           << expected << " - " << actual << ") < " << epsilon << "]";
        return ss.str();
    }
    return "";
}

/**
 * @param expected The expected expression
 * @param actual The actual expression
 * @param expstr The expected string
 * @param actstr The actual string
 */
template <class T, class K>
inline std::string assert_less(const T& expected, const K& actual,
                               const char* expstr, const char* actstr)
{
    if (!(expected < actual))
    {
        std::stringstream ss;
        ss << "[" << expstr << " < " << actstr << "] => [" << expected << " < "
           << actual << "]";
        return ss.str();
    }
    return "";
}

/**
 * @param expected The expected expression
 * @param actual The actual expression
 * @param expstr The expected string
 * @param actstr The actual string
 */
template <class T, class K>
inline std::string assert_greater(const T& expected, const K& actual,
                                  const char* expstr, const char* actstr)
{
    if (!(expected > actual))
    {
        std::stringstream ss;
        ss << "[" << expstr << " > " << actstr << "] => [" << expected << " > "
           << actual << "]";
        return ss.str();
    }
    return "";
}

/**
 * Exception class used to report errors in the unit test.
 */
class unit_test_exception: public std::runtime_error
{
    public:
        using std::runtime_error::runtime_error;
};

/**
 * Runs a unit test in a semi-controlled environment.
 * @param testName The name to display when running this test
 * @param func The function (unit test) to run. This function should take
 *  no parameters and return void.
 */
template <class Func>
int run_test(const std::string& test_name, Func&& func)
{
    try
    {
        func();
    }
    catch (std::exception& ex)
    {
        std::cerr << "        " << std::setw(40) << std::left
                  << (test_name + ": ") << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
}
}

#endif
