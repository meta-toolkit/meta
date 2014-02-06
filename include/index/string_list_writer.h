/**
 * @file string_list_writer.h
 * @author Chase Geigle
 */

#ifndef _META_STRING_LIST_WRITER_H_
#define _META_STRING_LIST_WRITER_H_

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
     * @param path the path to write the string file to.
     * @param size the number of strings in the list (must be known)
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
    // workaround for lack of move operators for gcc 4.8
    using ofstream = std::unique_ptr<std::ofstream>;
    std::ofstream& file()
    {
        return *string_file_;
    }
    ofstream make_file(const std::string& path)
    {
        return make_unique<std::ofstream>(path);
    }
#endif

    std::mutex mutex_;            /// writes are internally synchronized
    ofstream string_file_;        /// the file containing the strings
    uint64_t write_pos_;          /// keeps track of the write position
    util::disk_vector<uint64_t> index_; /// the index vector---stores byte positions
};
}
}

#endif
