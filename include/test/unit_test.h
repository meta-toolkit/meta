/**
 * @file unit_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <cmath>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
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
        std::cerr << "[ " << printing::make_red("FAIL") << " ] " << (why)      \
                  << " (";                                                     \
        std::cerr << __FILE__ << ":" << __LINE__ << ")" << std::endl;          \
        exit(1);                                                               \
    } while (0)

/**
 * Same as FAIL, except no line number is printed.
 * This is useful for failing after a segfault for example;
 *  the line number would be useless since it will report somewhere inside this
 * file.
 */
#define FAIL_NOLINE(why)                                                       \
    do                                                                         \
    {                                                                          \
        std::cerr << "[ " << printing::make_red("FAIL") << " ]"                \
                  << " " << (why) << std::endl;                                \
        exit(1);                                                               \
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
/**
 * Used to specify whether to fork() the function; useful for when you
 * want to step through with the debugger.
 */
static bool debug = false;

/**
 * Used to compare floating point equality
 */
static double epsilon = 0.0000001;

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
 * Signal handler for unit tests.
 * Catches signals and responds appropriately, usually by failing the
 *  current test.
 * @param sig The caught signal ID
 */
void sig_catch(int sig);

/**
 * Runs a unit test in a semi-controlled environment.
 * @param testName - the name to display when running this test
 * @param timeout - how long to allow this test to execute (in seconds).
 *  Default is one second.
 * @param func - the function (unit test) to run. This function should take
 *  no parameters and return void.
 */
template <class Func>
int run_test(const std::string& test_name, int timeout, Func&& func)
{
    std::cerr << std::left << std::setw(50) << (" " + test_name);

    if (debug)
    {
        func();
        std::cerr << "[ debug ] " << std::endl;
        return 0;
    }

    struct sigaction act;
    act.sa_handler = sig_catch;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGALRM, &act, 0);
    sigaction(SIGSEGV, &act, 0);
    sigaction(SIGINT, &act, 0);

    int status;
    pid_t pid = fork();
    if (pid == 0)
    {
        alarm(timeout);
        try
        {
            func();
        }
        catch (std::exception& ex)
        {
            std::string msg = " caught exception: " + std::string{ex.what()};
            FAIL(msg);
        }
        std::cerr << "[ " << printing::make_green("OK") << " ] " << std::endl;
        exit(0);
    }
    else if (pid > 0)
    {
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status))
        {
            FAIL(" child did not exit properly");
            return 1;
        }

        return WEXITSTATUS(status);
    }
    else
    {
        std::cerr << "[ " << printing::make_red("ERROR") << " ]"
                  << ": failure to fork" << std::endl;
    }

    return 1;
}

template <class Func>
int run_test(const std::string& test_name, Func&& func)
{
    return run_test(test_name, 1, func);
}

void report(int num_failed, bool done = false);
}
}

#endif
