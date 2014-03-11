/**
 * @file classifier_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFIER_TEST_H_
#define META_CLASSIFIER_TEST_H_

#include <fstream>
#include <iostream>
#include "test/unit_test.h"
#include "test/inverted_index_test.h"
#include "classify/classifier/all.h"
#include "classify/kernel/all.h"
#include "caching/all.h"
#include "index/ranker/all.h"

namespace meta
{
namespace testing
{

template <class Index, class Classifier>
void check_cv(Index& idx, Classifier& c, double min_accuracy);

template <class Index, class Classifier>
void check_split(Index& idx, Classifier& c, double min_accuracy);

int run_tests(const std::string& type);

int classifier_tests();

}
}

#endif
