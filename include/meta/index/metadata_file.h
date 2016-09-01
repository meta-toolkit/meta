/**
 * @file metadata_file.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_METADATA_FILE_H_
#define META_INDEX_METADATA_FILE_H_

#include "meta/config.h"
#include "meta/corpus/metadata.h"
#include "meta/io/mmap_file.h"
#include "meta/util/disk_vector.h"

namespace meta
{
namespace index
{

/**
 * Used for reading document-level metadata for an index.
 *
 * The following two-file format is used:
 *
 * - metadata.index: disk vector indexed by document id, denoting the seek
 *   position for each document's metdata in the metadata.db file.
 *
 * - metadata.db: <MDDB>
 *   - <MDDB> => <Header> <DocumentMD>^<NumDocs>
 *   - <Header> => <FieldCount> <FieldHeader>^(<FieldCount> + 2)
 *   - <FieldNum> => PackedInt
 *   - <FieldHeader> => <FieldName> <FieldType>
 *   - <FieldName> => String
 *   - <FieldType> => field_type
 *   - <DocumentMD> => <DocLength> <UniqueTerms> <UserMetadata>^FieldNum
 *   - <DocLength> => PackedInt
 *   - <UniqueTerms> => PackedInt
 *   - <UserMetaData> => PackedInt | PackedDouble | String (depending on
 *     <FieldType> in the metadata.index header)
 *
 * <FieldCount> is the number of user-supplied metadata fields (they must
 * be present for all documents). We add two in the grammar above since we
 * always represent the length (integer) and unique-terms (integer) as
 * metadata. The "length", "unique-terms", and "path" metadata names are
 * **reserved**, but there can be more metadata if the user supplies it.
 */
class metadata_file
{
  public:
    /**
     * Opens the metadata file stored at prefix.
     */
    metadata_file(const std::string& prefix);

    /**
     * Obtains metadata for a document. The object returned is a proxy and
     * will look up metadata upon first request. If metadata is requested
     * multiple times from the same metadata object, it will not be
     * re-parsed from the file.
     *
     * @param d_id The document id to look up metadata for
     * @return the metadata for the document
     */
    corpus::metadata get(doc_id d_id) const;

    /**
     * @return the number of documents in this database
     */
    uint64_t size() const;

  private:
    /// the schema for this file
    corpus::metadata::schema_type schema_;

    /// the seek positions for every document in this file
    util::disk_vector<uint64_t> index_;

    /// the mapped file for reading metadata from
    io::mmap_file md_db_;
};
}
}
#endif
