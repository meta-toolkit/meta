/**
 * @file parser.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <unordered_set>
#include "io/mmap_file.h"

namespace meta {
namespace io {

/**
 * Parses a text file by reading it completely into memory, delimiting tokens
 * by user request.
 */
class parser
{
    public:
        /**
         * Determines whether the parser parses a std::string or the contents
         * of a file
         */
        enum class input_type { File, String };

        /**
         * @param path The path to the file to parse
         * @param delims Delimiters to be used for separating tokens
         * @param in_type
         */
        parser(const std::string & input, const std::string & delims,
                input_type in_type = input_type::File);

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

        /** the current position of the "cursor" into the file or string */
        size_t _idx;

        /** invalid characters that serve as delimiters */
        std::unordered_set<char> _invalid;

        /** saves the name of the file if the parser is parsing a file */
        std::string _filename;

        /** memory-mapped file pointer if the parser is parsing a file */
        std::unique_ptr<io::mmap_file> _mmap_file;

        /** the number of characters that will be read */
        uint64_t _size;

        /** pointer into a string or memory-mapped file */
        const char* _data;

        /** the next token to be returned; "" if none */
        std::string _next;
};

}
}

#endif
