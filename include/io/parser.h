/**
 * @file parser.h
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
class Parser
{
    public:
        /**
         * @param path The path to the file to parse
         * @param delims Delimiters to be used for separating tokens
         */
        Parser(const std::string & path, const std::string & delims);

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
        bool hasNext() const;

    private:

        size_t _idx;

        std::string _filename;

        std::vector<std::string> _tokens;
};

}
}

#endif
