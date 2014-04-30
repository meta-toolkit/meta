/**
 * @file metapath_test.cpp
 * @author Sean Massung
 */

#include "graph/metapath.h"
#include "test/unit_test.h"
#include "test/metapath_test.h"

namespace meta
{
namespace testing
{
int metapath_tests()
{
    int num_failed = 0;

    num_failed += testing::run_test("simple", [&]()
    {
        graph::metapath mpath1{"A -- P -- A"};
        ASSERT_EQUAL(mpath1.text(), "A -- P -- A");
        mpath1.reverse();
        ASSERT_EQUAL(mpath1.text(), "A -- P -- A");

        graph::metapath mpath2{"A -> P -- A"};
        ASSERT_EQUAL(mpath2.text(), "A -> P -- A");
        mpath2.reverse();
        ASSERT_EQUAL(mpath2.text(), "A -- P <- A");

        graph::metapath mpath3{"A -- P -> P -> V"};
        ASSERT_EQUAL(mpath3.text(), "A -- P -> P -> V");
        mpath3.reverse();
        ASSERT_EQUAL(mpath3.text(), "V <- P <- P -- A");
    });

    return num_failed;
}
}
}
