/**
 * @file algorithms.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_ALGORITHMS_ALL_H_
#define META_GRAPH_ALGORITHMS_ALL_H_

#include <stdexcept>

namespace meta
{
namespace graph
{
namespace algorithms
{
/**
 * Exception for errors in graph algorithms.
 */
class graph_algorithm_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

}
}
}

#include "meta/graph/algorithms/search.h"
#include "meta/graph/algorithms/model.h"
#include "meta/graph/algorithms/measure.h"
#include "meta/graph/algorithms/centrality.h"

#endif
