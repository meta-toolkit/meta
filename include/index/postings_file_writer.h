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

#include "io/compressed_file_writer.h"
#include "util/disk_vector.h"

namespace meta
{
namespace index
{

class postings_file_writer
{
  public:
    /**
     * Opens a postings file for writing.
     * @param filename The filename (prefix) for the postings file.
     */
    postings_file_writer(const std::string& filename, uint64_t unique_keys)
        : output_{filename, io::default_compression_writer_func},
          bit_locations_{filename + "_index", unique_keys},
          id_{0}
    {
        // nothing
    }

    /**
     * Writes a postings data object to the file.
     * @param pdata The postings_data to be written
     */
    template <class PostingsData>
    void write(const PostingsData& pdata)
    {
        bit_locations_[id_] = output_.bit_location();
        pdata.write_compressed(output_);
        ++id_;
    }

  private:
    io::compressed_file_writer output_;
    util::disk_vector<uint64_t> bit_locations_;
    uint64_t id_;
};
}
}
#endif
