/**
 * @file undirected_graph_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UNDIRECTED_GRAPH_TEST_H_
#define META_UNDIRECTED_GRAPH_TEST_H_

#include "graph/undirected_graph.h"
#include "test/unit_test.h"

namespace meta
{
namespace testing
{
/**
 * Runs the undirected graph tests.
 * @return the number of tests failed.
 */
int undirected_graph_tests();
}
}

#endif
