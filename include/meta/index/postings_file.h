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

#include "meta/config.h"
#include "meta/index/postings_data.h"
#include "meta/index/postings_stream.h"
#include "meta/io/mmap_file.h"
#include "meta/util/disk_vector.h"
#include "meta/util/optional.h"

namespace meta
{
namespace index
{

/**
 * File that stores the postings list for an index on disk. Each postings
 * list is indexed via PrimaryKey and consists of pairs of (SecondaryKey,
 * double).
 */
template <class PrimaryKey, class SecondaryKey, class FeatureValue = uint64_t>
class postings_file
{
  public:
    using postings_data_type
        = postings_data<PrimaryKey, SecondaryKey, FeatureValue>;

    /**
     * Opens a postings file.
     * @param filename The path to the file
     */
    postings_file(const std::string& filename)
        : postings_{filename}, byte_locations_{filename + "_index"}
    {
        // nothing
    }

    /**
     * Obtains a postings stream object for the given primary key.
     * @param pk The primary key to look up
     * @return a postings stream for this primary key, if it is in the
     * postings file
     */
    util::optional<postings_stream<SecondaryKey, FeatureValue>>
    find_stream(PrimaryKey pk) const
    {
        if (pk < byte_locations_.size())
            return postings_stream<SecondaryKey, FeatureValue>{
                postings_.begin() + byte_locations_.at(pk)};
        return util::nullopt;
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
        if (idx < byte_locations_.size())
        {
            auto stream = find_stream(pk);
            pdata->set_counts(stream->begin(), stream->end());
        }

        return pdata;
    }

  private:
    io::mmap_file postings_;
    util::disk_vector<uint64_t> byte_locations_;
};
}
}
#endif
