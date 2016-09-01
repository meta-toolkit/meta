/**
 * @file postings_file_writer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_POSTINGS_FILE_WRITER_H_
#define META_INDEX_POSTINGS_FILE_WRITER_H_

#include <fstream>
#include <numeric>

#include "meta/config.h"
#include "meta/io/packed.h"
#include "meta/util/disk_vector.h"

namespace meta
{
namespace index
{

template <class PostingsData>
class postings_file_writer
{
  public:
    /**
     * Opens a postings file for writing.
     * @param filename The filename (prefix) for the postings file.
     */
    postings_file_writer(const std::string& filename, uint64_t unique_keys)
        : output_{filename, std::ios::binary},
          byte_locations_{filename + "_index", unique_keys},
          byte_pos_{0},
          id_{0}
    {
        // nothing
    }

    /**
     * Writes a postings data object to the file.
     *
     * @param pdata The postings_data to be written
     */
    void write(const PostingsData& pdata)
    {
        byte_locations_[id_] = byte_pos_;
        byte_pos_ += pdata.write_packed_counts(output_);
        ++id_;
    }

  private:
    std::ofstream output_;
    util::disk_vector<uint64_t> byte_locations_;
    uint64_t byte_pos_;
    uint64_t id_;
};
}
}
#endif
