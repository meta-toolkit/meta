/**
 * @file parser.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_PARSER_H_
#define META_PARSER_H_

#include <array>
#include <memory>
#include <string>
#include "util/optional.h"

namespace meta
{
namespace io
{

class mmap_file;

/**
 * Parses a text file by reading it completely into memory, delimiting tokens
 * by user request.
 */
class parser
{
  public:
    /**
     * Determines whether the parser parses a std::string or the contents
     * of a file.
     */
    enum class input_type
    {
        File,
        String
    };

    /**
     * @param path The path to the file to parse
     * @param delims Delimiters to be used for separating tokens
     * @param in_type Determines whether the input is a file or a string
     */
    parser(const std::string& input, const std::string& delims,
           input_type in_type = input_type::File);

    /**
     * Destructor.
     */
    ~parser();

    /**
     * May be move-constructed.
     */
    parser(parser&&);

    /**
     * May be move-assigned.
     */
    parser& operator=(parser&&);

    /**
     * @return the filename of the file that is being parsed
     */
    std::string filename() const;

    /**
     * @return the next token in the parser without advancing the current
     * position
     */
    std::string peek() const;

    /**
     * @return the next token in the parser, advancing to the next one if
     * it exists
     */
    std::string next();

    /**
     * @return whether the parser contains another token
     */
    bool has_next() const;

  private:
    /**
     * Advances to the next token in the file or string, saving the result.
     */
    void get_next();

    /// The current position of the "cursor" into the file or string
    size_t idx_;

    /// Array of booleans indicating whether or not a character is a delimiter
    std::array<bool, 256> invalid_;

    /// Saves the name of the file if the parser is parsing a file
    std::string filename_;

    /// Memory-mapped file pointer if the parser is parsing a file
    std::unique_ptr<io::mmap_file> mmap_file_;

    /// The number of characters that will be read
    uint64_t size_;

    /// Pointer into a string or memory-mapped file
    const char* data_;

    /// The next token to be returned; "" if none
    util::optional<std::string> next_;
};
}
}

#endif
