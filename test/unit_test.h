/**
 * @file unit_test.h
 *
 * Contains functions and defines used in unit testing.
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <iomanip>
#include <string>
#include <iostream>
#include "util/common.h"

/**
 * Fail if expr is false; otherwise continue.
 */
#define ASSERT(expr) \
    do { \
       if (!(expr)) \
           FAIL("Assertion failed: " #expr ); \
    } while(0)

/**
 * Fail this test case with an explanation.
 * Give line number and file where the test case failed.
 */
#define FAIL(why) \
    do { \
        std::cerr << "[ " << common::make_red("FAIL") << " ] " << (why) << " ("; \
        std::cerr << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        exit(1); \
    } while(0)

/**
 * Same as FAIL, except no line number is printed.
 * This is useful for failing after a segfault for example;
 *  the line number would be useless since it will report somewhere inside this file.
 */
#define FAIL_NOLINE(why) \
    do { \
        std::cerr << "[ " << common::make_red("FAIL") << " ]" \
        << " " << (why) << std::endl; \
        exit(1); \
    } while(0)

namespace meta {

/**
 * Contains unit testing functions for the META toolkit.
 */
namespace testing
{
    /**
     * Signal handler for unit tests.
     * Catches signals and responds appropriately, usually by failing the
     *  current test.
     * @param sig The caught signal ID
     */
    void sig_catch(int sig)
    {
        switch(sig)
        {
            case SIGALRM: FAIL_NOLINE("Time limit exceeded"); break;
            case SIGSEGV: FAIL_NOLINE("Received segfault"); break;
            case SIGINT:  FAIL_NOLINE("Received interrupt, exiting."); break;
        }
    }

    /**
     * Runs a unit test in a semi-controlled environment.
     * @param testName - the name to display when running this test
     * @param timeout - how long to allow this test to execute (in seconds).
     *  Default is one second.
     * @param func - the function (unit test) to run. This function should take
     *  no parameters and return void.
     */
    template <class Func>
    void run_test(const std::string & test_name, int timeout, Func && func)
    {
        std::cerr << std::left << std::setw(30) << (" " + test_name);

        struct sigaction act;
        act.sa_handler = sig_catch;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

        sigaction(SIGALRM, &act, 0);
        sigaction(SIGSEGV, &act, 0);
        sigaction(SIGINT, &act, 0);

        pid_t pid = fork();
        if(pid == 0)
        {
            alarm(timeout);
            func();
            std::cerr << "[ " << common::make_green("OK") << " ] " << std::endl;
            exit(0);
        }
        else if(pid > 0)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            std::cerr << "[ " << common::make_red("ERROR") << " ]"
                 << ": failure to fork" << std::endl;
        }
    }

    template <class Func>
    void run_test(const std::string & test_name, Func && func)
    {
        run_test(test_name, 1, func);
    }
}
}

#endif
