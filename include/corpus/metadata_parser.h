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

#include <fstream>

#include "corpus/metadata.h"
#include "util/optional.h"

namespace meta
{
namespace corpus
{

/**
 * Reads metadata from the metadata file of a corpus according to a schema.
 */
class metadata_parser
{
  public:
    /**
     * Creates the parser.
     * @param filename The name of the file to parse
     * @param schema The schema to parse the file with
     */
    metadata_parser(const std::string& filename, metadata::schema schema);

    /**
     * @return the metadata vector for the next document in the file
     */
    std::vector<metadata::field> next();

    /**
     * @return the schema for the metadata in this file
     */
    const metadata::schema& schema() const;

  private:
    /// the parser used to extract metadata
    /// unique_ptr because GCC<5.0 can't move streams
    std::unique_ptr<std::ifstream> infile_;

    /// the schema for the metadata being extracted
    metadata::schema schema_;
};
}
}
#endif
