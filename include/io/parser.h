/**
 * @file parser.h
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <vector>
#include <string>

class Parser
{
    public:
        /**
         *
         */
        Parser(const std::string & path, const std::string & delims);

        /**
         *
         */
        std::string filename() const;

        /**
         *
         */
        std::string peek() const;

        /**
         *
         */
        std::string next();
        
        /**
         *
         */
        bool hasNext() const;

    private:

        size_t _idx;

        std::string _filename;

        std::vector<std::string> _tokens;
};

#endif
