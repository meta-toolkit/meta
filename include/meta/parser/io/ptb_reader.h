/**
 * @file ptb_reader.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_PTB_READER_H_
#define META_PARSER_PTB_READER_H_

#include <istream>
#include <vector>
#include "meta/parser/trees/parse_tree.h"

namespace meta
{
namespace parser
{
namespace io
{

/**
 * Reads a Penn Treebank formatted tree file and returns a set of
 * trees parsed from it.
 *
 * We are assuming here that the trees being read are also POS-tagged
 * (e.g., are from the mrg/ folder in the distribution).
 *
 * @param filename The name of the file to be parsed
 * @return all of the trees that were read from the given file
 */
std::vector<parse_tree> extract_trees(const std::string& filename);

/**
 * Reads Penn Treebank formatted trees from a stream and returns a set of
 * trees parsed from it. This overload can be used both for reading from
 * files as well as reading from standard in, strings (via stringstream),
 * etc.
 *
 * We are assuming here that the trees being read are also POS-tagged
 * (e.g., are from the mrg/ folder in the distribution).
 *
 * @param filename The name of the file to be parsed
 * @return all of the trees that were read from the given stream
 */
std::vector<parse_tree> extract_trees(std::istream& stream);

}
}
}
#endif
