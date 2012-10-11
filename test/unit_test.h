/**
 * @file unit_test.h
 *
 * Contains macros used in unit testing.
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

#define PASS \
    do { \
        std::cerr << Common::makeGreen("OK") << std::endl; \
        exit(0); \
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
    void sigCatch(int sig)
    {
        switch(sig)
        {
            case SIGALRM:
                FAIL("Time limit exceeded");
                break;
            case SIGSEGV:
                FAIL("Received segfault");
                break;
        }
    }

    /**
     *
     */
    template <class Func>
    void runTest(const string & testName, Func func, int timeout = 2)
    {
        cerr << " " << testName << "... ";

        struct sigaction act;
        act.sa_handler = sigCatch;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGALRM, &act, 0);
        sigaction(SIGSEGV, &act, 0);

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
