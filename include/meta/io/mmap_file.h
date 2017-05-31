/**
 * @file mmap_file.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_MMAP_FILE_H_
#define META_MMAP_FILE_H_

#include <stdexcept>
#include <string>

#include "meta/config.h"
#include "meta/util/optional.h"

namespace meta
{
namespace io
{

/**
 * Memory maps a text file readonly.
 */
class mmap_file
{
  public:
    /**
     * Constructor.
     * @param path Path to the text file to open
     */
    mmap_file(const std::string& path);

    /**
     * Move constructor.
     */
    mmap_file(mmap_file&&);

    /**
     * Move assignment operator.
     */
    mmap_file& operator=(mmap_file&&);

    /**
     * Destructor; deallocates memory used to store this object, closing the
     * text file.
     */
    ~mmap_file();

    /**
     * @param index The byte in the memory-mapped file to access
     * @return the requested byte; an exception is thrown if the index is
     * out of bounds of the mapped region
     */
    char operator[](uint64_t index) const;

    /**
     * @return the length of the file in bytes
     */
    uint64_t size() const;

    /**
     * @return the title of the text file (the parameter given to the
     * contructor)
     */
    std::string path() const;

    /**
     * @return a pointer to the beginning of the file
     */
    char* begin() const;

  private:
    /// Filename of the text file
    std::string path_;

    /// Pointer to the beginning of the text file
    char* start_;

    /// Size of the current text file
    uint64_t size_;

    /// File descriptor for the open text file
    int file_descriptor_;

    /// No copying */
    mmap_file(const mmap_file& other) = delete;

    /// no copying */
    const mmap_file& operator=(const mmap_file& other) = delete;
};

/**
 * Basic exception for mmap_file interactions.
 */
class mmap_file_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * A stream for use with io::packed that reads from a memory mapped file.
 */
class mmap_ifstream
{
  public:
    mmap_ifstream() = default;
    mmap_ifstream(mmap_ifstream&&) = default;
    mmap_ifstream& operator=(mmap_ifstream&&) = default;
    mmap_ifstream(const std::string& filename);

    bool is_open() const;
    int peek() const;
    int get();
    void close();

  private:
    util::optional<mmap_file> file_;
    std::size_t pos_;
};
}
}

#endif
