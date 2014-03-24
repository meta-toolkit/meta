/**
 * @file ptb_parser.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PTB_PARSER_H_
#define META_PTB_PARSER_H_

#include "sequence/sequence.h"

namespace meta
{
namespace sequence
{

/**
 * Reads a Penn Treebank formatted "combined" file, extracting the
 * POS-tagged sequences from it by parsing the trees.
 */
std::vector<sequence> extract_sequences(const std::string& filename);

}
}
#endif
