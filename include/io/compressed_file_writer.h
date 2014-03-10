/**
 * @file compressed_file_writer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_COMPRESSED_FILE_WRITER_H_
#define META_COMPRESSED_FILE_WRITER_H_

#include <functional>
#include <stdexcept>
#include <string>

namespace meta
{
namespace io
{

/**
 * Writes to a file of unsigned integers using gamma compression.
 */
class compressed_file_writer
{
  public:
    /**
     * Constructor; Opens a compressed file for writing or creates a new
     * file if it doesn't exist.
     * @param filename The path to the compressed file
     * @param mapping A function to map the original numbers to their
     * compressed id, usually to take advantage of a skewed distribution of
     * towards many small numbers
     */
    compressed_file_writer(const std::string& filename,
                           std::function<uint64_t(uint64_t)> mapping);

    /**
     * Destructor; closes the compressed file.
     */
    ~compressed_file_writer();

    /**
     * @return the character index and bit index of the current location in
     * the compressed file
     */
    uint64_t bit_location() const;

    /**
     * Writes a value to the end of the compressed file.
     * @param value The number to write
     */
    void write(uint64_t value);

    /**
     * Writes a binary string to the file.
     * @param str The string to write
     */
    void write(const std::string& str);

    /**
     * Closes this compressed file.
     */
    void close();

  private:
    /**
     * Writes a bit to the file and advances the current location in the
     * file.
     * @param bit
     */
    void write_bit(bool bit);

    /**
     * Writes the buffer to the file.
     */
    void write_buffer() const;

    /** where to write the compressed data */
    FILE* outfile_;

    /** the current byte this reader is on */
    uint64_t char_cursor_;

    /** the current bit of the current byte this reader is on */
    uint64_t bit_cursor_;

    /** how large to make the internal writer buffer */
    uint64_t buffer_size_;

    /** saved data that is not yet written to disk */
    unsigned char* buffer_;

    /** the mapping to use (actual -> compressed id) */
    std::function<uint64_t(uint64_t)> mapping_;

    /** the number of total bits that have been written (for seeking )*/
    uint64_t bit_location_;

    /** ensures the file isn't closed more than once */
    bool closed_;

  public:
    /**
     * Basic exception for compressed_file_writer interactions.
     */
    class compressed_file_writer_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};

uint64_t default_compression_writer_func(uint64_t key);
}
}

#endif
