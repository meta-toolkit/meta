/**
 * @file metadata_writer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_METADATA_WRITER_H_
#define META_INDEX_METADATA_WRITER_H_

#include <mutex>

#include "meta/config.h"
#include "meta/corpus/document.h"
#include "meta/corpus/metadata.h"
#include "meta/util/disk_vector.h"

namespace meta
{
namespace index
{

/**
 * Writes document metadata into the packed format for the index.
 */
class metadata_writer
{
  public:
    /**
     * Constructs the writer.
     * @param prefix The directory to place the metadata database and index
     * @param num_docs The number of documents we have metadata for
     * @param schema The schema for the metadata we will store
     */
    metadata_writer(const std::string& prefix, uint64_t num_docs,
                    corpus::metadata::schema_type schema);

    /**
     * Writes a document's metadata to the database and index.
     * @param d_id The document id
     * @param length The length of the document
     * @param num_unique The number of unique terms in the document
     * @param mdata Any additional metadata to be written
     */
    void write(doc_id d_id, uint64_t length, uint64_t num_unique,
               const std::vector<corpus::metadata::field>& mdata);

  private:
    /// a lock for thread safety
    std::mutex lock_;

    /// the index into the database file
    util::disk_vector<uint64_t> seek_pos_;

    /// the current byte position in the database
    uint64_t byte_pos_;

    /// the output stream for the database file
    std::ofstream db_file_;

    /// the schema of the metadata we are writing
    corpus::metadata::schema_type schema_;
};
}
}
#endif
