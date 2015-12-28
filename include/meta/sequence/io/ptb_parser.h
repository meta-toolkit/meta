/**
 * @file ptb_parser.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PTB_PARSER_H_
#define META_PTB_PARSER_H_

#include "meta/sequence/sequence.h"

namespace meta
{
namespace sequence
{

/**
 * Reads a Penn Treebank formatted part of speech tagged file and returns
 * a set of sequences parsed from it.
 *
 * @param filename The name of the file to be parsed
 * @return all of the sequences that were parsed from the given file
 */
std::vector<sequence> extract_sequences(const std::string& filename);

}
}
#endif
