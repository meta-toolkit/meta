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
#include "corpus/document.h"
#include "corpus/metadata.h"
#include "util/disk_vector.h"

namespace meta
{
namespace index
{

class metadata_writer
{
  public:
    metadata_writer(const std::string& prefix, uint64_t num_docs,
                    corpus::metadata::schema schema);

    void write(doc_id d_id, uint64_t length, uint64_t num_unique,
               const std::vector<corpus::metadata::field>& mdata);

  private:
    std::mutex lock_;
    util::disk_vector<uint64_t> seek_pos_;
    uint64_t byte_pos_;
    std::ofstream db_file_;
    corpus::metadata::schema schema_;
};
}
}
#endif
