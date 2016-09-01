/**
 * @file libsvm_parser.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LIBSVM_PARSER_H_
#define META_LIBSVM_PARSER_H_

#include <string>
#include <utility>
#include <vector>

#include "meta/config.h"
#include "meta/meta.h"

namespace meta
{
namespace io
{
namespace libsvm_parser
{
/// Collection of (term_id, double)
using counts_t = const std::vector<std::pair<term_id, double>>;

/**
 * Extracts a class_label from a string in libsvm format
 * @param text A libsvm-formatted string; throws an exception if it can't
 * be parsed correctly
 * @return the class_label
 */
class_label label(const std::string& text);

/**
 * @param text A libsvm-formatted string; throws an exception if it can't
 * be parsed correctly
 * @param contains_label Whether this string's first token is the
 * class_label
 * @return (feature, count) pairs of the libsvm data
 */
counts_t counts(const std::string& text, bool contains_label = true);

/**
 * Exception class for this parser.
 */
class libsvm_parser_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
}

#endif
