/**
 * @file metadata_parser.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CORPUS_METADATA_PARSER_H_
#define META_CORPUS_METADATA_PARSER_H_

#include "corpus/metadata.h"
#include "io/parser.h"
#include "util/optional.h"

namespace meta
{
namespace corpus
{

class metadata_parser
{
  public:
    metadata_parser(const std::string& filename,
                    metadata::schema schema);

    std::vector<metadata::field> next();

    const metadata::schema& schema() const;

  private:
    util::optional<io::parser> parser_;
    metadata::schema schema_;
};
}
}
#endif
