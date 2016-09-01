/**
 * @file chunk.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CHUNK_H_
#define META_CHUNK_H_

#include <cstdint>
#include <string>

#include "meta/config.h"

namespace meta
{
namespace index
{

/**
 * Represents a portion of a disk_index's postings file. It is an intermediate
 * file mapping primary keys to secondary keys. The chunks are sorted to enable
 * efficient merging, and define an operator< to allow them to be sorted or
 * stored in a priority queue.
 */
template <class PrimaryKey, class SecondaryKey>
class chunk
{
  public:
    /**
     * @param path The path to this chunk file on disk
     */
    chunk(const std::string& path);

    /**
     * @param other The other chunk to compare with this one
     * @return whether this chunk is less than (has a smaller size than)
     * the parameter
     */
    bool operator<(const chunk& other) const;

    /**
     * @return the size of this postings file chunk in bytes
     */
    uint64_t size() const;

    /**
     * @return the path to this chunk
     */
    std::string path() const;

    /**
     * @param pdata A collection of postings data to combine with this chunk
     * pdata must:
     *  - support iteration in sorted order
     *  - dereferenced type must be a
     *      postings_data<PrimaryKey, SecondaryKey> object
     *  - implement the clear() function
     */
    template <class Container>
    void memory_merge_with(Container& pdata);

  private:
    /// Calculates the size of the file this chunk represents in bytes.
    void set_size();

    /// The path to this chunk file on disk
    std::string path_;

    /// The number of bytes this chunk takes up
    uint64_t size_;
};
}
}

#include "meta/index/chunk.tcc"
#endif
