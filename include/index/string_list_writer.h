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

#if !META_HAS_STREAM_MOVE
#include <memory>
#include "util/shim.h"
#endif

#include "util/disk_vector.h"

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
#if META_HAS_STREAM_MOVE
    using ofstream = std::ofstream;
    std::ofstream& file()
    {
        return string_file_;
    }
    ofstream make_file(const std::string& path)
    {
        return std::ofstream{path};
    }
#else
    /// workaround for lack of move operators for gcc 4.8
    using ofstream = std::unique_ptr<std::ofstream>;
    /**
     * @return a reference to the file stream
     */
    std::ofstream& file()
    {
        return *string_file_;
    }
    /**
     * @param path The path to the file
     * @return a std::ofstream created from the file
     */
    ofstream make_file(const std::string& path)
    {
        return make_unique<std::ofstream>(path);
    }
#endif

    /// Writes are internally synchronized
    std::mutex mutex_;

    /// The file containing the strings
    ofstream string_file_;

    /// Keeps track of the write position
    uint64_t write_pos_;

    /// Index vector---stores byte positions
    util::disk_vector<uint64_t> index_;
};
}
}

#endif
