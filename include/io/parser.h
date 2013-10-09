/**
 * @file parser.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <vector>
#include <string>

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

        void parse_file(const std::string & filename,
                const std::string & delims);

        void parse_string(const std::string & input,
                const std::string & delims);

        size_t _idx;

        std::string _filename;

        std::vector<std::string> _tokens;
};

}
}

#endif
