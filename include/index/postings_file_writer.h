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
#include "io/packed.h"
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
        : output_{filename, std::ios::binary},
          byte_locations_{filename + "_index", unique_keys},
          byte_pos_{0},
          id_{0}
    {
        // nothing
    }

    /**
     * Writes a postings data object to the file.
     * @param pdata The postings_data to be written
     */
    template <class FeatureValue = uint64_t, class PostingsData>
    void write(const PostingsData& pdata)
    {
        byte_locations_[id_] = byte_pos_;
        byte_pos_ += io::packed::write(output_, pdata.counts().size());

        auto total_counts = std::accumulate(
            pdata.counts().begin(), pdata.counts().end(), uint64_t{0},
            [](uint64_t cur, const typename PostingsData::pair_t& pr)
            {
                return cur + static_cast<uint64_t>(pr.second);
            });
        byte_pos_ += io::packed::write(output_, total_counts);

        uint64_t last_id = 0;
        for (const auto& count : pdata.counts())
        {
            byte_pos_ += io::packed::write(output_, count.first - last_id);

            if (std::is_same<FeatureValue, uint64_t>::value)
            {
                byte_pos_ += io::packed::write(
                    output_, static_cast<uint64_t>(count.second));
            }
            else
            {
                byte_pos_ += io::packed::write(output_, count.second);
            }

            last_id = count.first;
        }
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
