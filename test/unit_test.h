/**
 * @file unit_test.h
 *
 * Contains functions and defines used in unit testing.
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <iostream>
#include "util/common.h"

#define ASSERT(expr) \
    do { \
       if (!(expr)) \
           FAIL("Assertion failed: " #expr ); \
    } while(0)

#define FAIL(why) \
    do { \
        std::cerr << Common::makeRed("FAIL") << " " << (why) << " ("; \
        std::cerr << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        exit(1); \
    } while(0)

#define FAIL_NOLINE(why) \
    do { \
        std::cerr << Common::makeRed("FAIL") << " " << (why) << std::endl; \
        exit(1); \
    } while(0)

#define PASS \
    do { \
        std::cerr << Common::makeGreen("OK") << std::endl; \
        exit(0); \
    } while(0)

/**
 * Contains unit testing related functions.
 */
namespace UnitTests
{
    using std::string;
    using std::endl;
    using std::cerr;

    /**
     * Signal handler for unit tests.
     * Catches signals and responds appropriately, usually by failing the
     *  current test.
     * @param sig - the caught signal ID
     */
    void sigCatch(int sig)
    {
        switch(sig)
        {
            case SIGALRM: FAIL_NOLINE("Time limit exceeded");
            case SIGSEGV: FAIL_NOLINE("Received segfault");
            case SIGINT:  FAIL_NOLINE("Received interrupt, exiting.");
        }
    }

    /**
     * Runs a unit test in a semi-controlled environment.
     * @param testName - the name to display when running this test
     * @param func - the function (unit test) to run. This function should take
     *  no parameters and return void.
     * @param timeout - how long to allow this test to execute (in seconds).
     *  Default is one second.
     */
    template <class Func>
    void runTest(const string & testName, Func func, int timeout = 1)
    {
        cerr << " " << testName << "... ";

        struct sigaction act;
        act.sa_handler = sigCatch;
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
        }
        else if(pid > 0)
            waitpid(pid, NULL, 0);
        else
            cerr << Common::makeRed("ERROR") << ": failure to fork" << endl;
    }
}

#endif
