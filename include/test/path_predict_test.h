/**
 * @file path_predict_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_PATH_PREDICT_TEST_H_
#define META_PATH_PREDICT_TEST_H_

#include "graph/algorithm/metapath_measures.h"

namespace meta
{
namespace testing
{
/**
 * Runs the path_predict and metapath_measures tests using the small example
 * DBLP dataset found in Figure 3 of
 * @see Co-Author Relationship Prediction in Heterogeneous Bibliographic
 * Networks, Sun et. al. 2011.
 * @return the number of tests failed.
 */
int path_predict_tests();
}
}

#endif
