/**
 * @file unit_test.cpp
 * @author Sean Massung
 */

#include "test/unit_test.h"

namespace meta
{

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
    switch (sig)
    {
        case SIGALRM:
            FAIL_NOLINE("Time limit exceeded");
            break;
        case SIGSEGV:
            FAIL_NOLINE("Received segfault");
            break;
        case SIGINT:
            FAIL_NOLINE("Received interrupt, exiting.");
            break;
    }
}

void report(int num_failed, bool done)
{
    std::string msg;
    if (num_failed == 0)
    {
        msg = "[ " + printing::make_green("all tests passed") + " ]";
    }
    else
    {
        std::string tests{" test"};
        if (num_failed > 1)
            tests += "s";
        msg = "[ " + printing::make_red(std::to_string(num_failed) + tests
                                        + " failed") + " ]";
    }

    if (done)
        std::cerr << std::endl << printing::make_bold("Unit testing complete")
                  << std::endl;

    std::cerr << std::left << std::setw(50) << " ";
    std::cerr << msg << std::endl;
}
}
}
