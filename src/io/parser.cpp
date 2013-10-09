/**
 * @file parser.cpp
 */

#include <unordered_set>
#include "io/parser.h"
#include "io/mmap_file.h"

namespace meta {
namespace io {

parser::parser(const std::string & input, const std::string & delims,
        input_type in_type /* = File */):
    _idx{0}
{
    if(in_type == input_type::File)
        parse_file(input, delims);
    else
        parse_string(input, delims);
}

void parser::parse_string(const std::string & input, const std::string & delims)
{
    _filename = "";
    std::unordered_set<char> invalid{delims.begin(), delims.end()};

    size_t left = 0;
    for(size_t i = 0; i < input.size(); ++i)
    {
        if(invalid.find(input[i]) != invalid.end())
        {
            // TODO: make sure this calculation is correct
            if(left != i)
                _tokens.push_back(input.substr(left, i - left));
            left = i + 1;
        }
    }

}

void parser::parse_file(const std::string & filename,
        const std::string & delims)
{
    _filename = filename;
    std::unordered_set<char> invalid{delims.begin(), delims.end()};

    mmap_file mmap_file{filename};
    char* file = mmap_file.start();
    
    size_t left = 0;
    for(size_t i = 0; i < mmap_file.size(); ++i)
    {
        if(invalid.find(file[i]) != invalid.end())
        {
            // only create a string object once we know the exact size and
            // content
            if(left != i)
                _tokens.push_back(std::string(file + left, i - left));
            left = i + 1;
        }
    }
}

std::string parser::filename() const
{
    return _filename;
}

std::string parser::peek() const
{
    return _tokens.at(_idx);
}

std::string parser::next()
{
    return _tokens[_idx++];
}

bool parser::has_next() const
{
    return _idx < _tokens.size();
}

}
}
