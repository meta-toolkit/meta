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
            if(left != i)
                _tokens.emplace_back(input.substr(left, i - left));
            left = i + 1;
        }
    }

    // get last token if there is no delimiter at the EOF
    if(left != input.size())
        _tokens.emplace_back(input.substr(left));
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
                _tokens.emplace_back(file + left, i - left);
            left = i + 1;
        }
    }

    // get last token if there is no delimiter at the EOF
    if(left != mmap_file.size())
        _tokens.emplace_back(left, mmap_file.size() - left);
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
