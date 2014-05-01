/**
 * @file dblp_loader.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DBLP_LOADER_H_
#define META_DBLP_LOADER_H_

#include <fstream>
#include <sstream>
#include "graph/directed_graph.h"
#include "graph/dblp_node.h"
#include "io/parser.h"

namespace meta
{
namespace graph
{
namespace dblp_loader
{
/**
 * @param g Graph object populate
 * @param prefix Path to the input files
 * @param start_year The start year (inclusive) to create the graph from (by
 * paper publish year)
 * @param end_year The end year (inclusive) to create the graph from (by paper
 * publish year)
 */
void load(graph::directed_graph<graph::dblp_node>& g, const std::string& prefix,
          uint64_t start_year = 0,
          uint64_t end_year = std::numeric_limits<uint64_t>::max());
}
}
}

#endif
