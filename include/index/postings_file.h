/**
 * @file postings_file.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_POSTINGS_FILE_H_
#define META_INDEX_POSTINGS_FILE_H_

#include "index/postings_data.h"
#include "io/mmap_file.h"
#include "util/disk_vector.h"

namespace meta
{
namespace index
{

template <class PrimaryKey, class SecondaryKey>
class postings_file
{
  public:
    using postings_data_type = postings_data<PrimaryKey, SecondaryKey>;

    /**
     * Opens a postings file.
     * @param filename The path to the file
     */
    postings_file(const std::string& filename)
        : postings_{filename}, bit_locations_{filename + "_index"}
    {
        // nothing
    }

    /**
     * Obtains a postings data object for the given primary key.
     * @param pk The primary key to look up
     * @return a shared pointer to the postings data extracted from the
     * file
     */
    std::shared_ptr<postings_data_type> find(PrimaryKey pk) const
    {
        auto pdata = std::make_shared<postings_data_type>(pk);
        uint64_t idx{pk};

        // if we are in-bounds of the postings file, populate counts
        if (idx < bit_locations_.size())
        {
            io::compressed_file_reader reader{
                postings_, io::default_compression_reader_func};
            reader.seek(bit_locations_.at(idx));

            pdata->read_compressed(reader);
        }

        return pdata;
    }

  private:
    io::mmap_file postings_;
    util::disk_vector<uint64_t> bit_locations_;
};
}
}
#endif
