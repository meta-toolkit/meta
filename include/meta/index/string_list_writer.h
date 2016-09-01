/**
 * @file string_list_writer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_STRING_LIST_WRITER_H_
#define META_STRING_LIST_WRITER_H_

#include <fstream>
#include <mutex>
#include <string>

#include "meta/config.h"
#include "meta/io/moveable_stream.h"
#include "meta/util/disk_vector.h"

namespace meta
{
namespace index
{

/**
 * A class for writing large lists of strings to disk with an associated
 * index file for fast random access. This class is used for writing the
 * output format read by the string_list class.
 */
class string_list_writer
{
  public:
    /**
     * Constructs the writer, writing the string file to the given path.
     * The index file will go alongside that path.
     *
     * @param path The path to write the string file to.
     * @param size The number of strings in the list (must be known)
     */
    string_list_writer(const std::string& path, uint64_t size);

    /**
     * May be move constructed.
     */
    string_list_writer(string_list_writer&&);

    /**
     * May be move assigned.
     */
    string_list_writer& operator=(string_list_writer&&);

    /**
     * Sets the string at idx to be elem.
     * @param idx
     * @param elem
     */
    void insert(uint64_t idx, const std::string& elem);

  private:
    /// Writes are internally synchronized
    std::mutex mutex_;

    /// The file containing the strings
    io::mofstream string_file_;

    /// Keeps track of the write position
    uint64_t write_pos_;

    /// Index vector---stores byte positions
    util::disk_vector<uint64_t> index_;
};
}
}

#endif
