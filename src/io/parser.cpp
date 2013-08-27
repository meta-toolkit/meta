/**
 * @file parser.cpp
 */

#include <unordered_set>
#include "io/parser.h"
#include "io/mmap_file.h"

namespace meta {
namespace io {

parser::parser(const std::string & path, const std::string & delims):
    _idx{0},
    _filename{path}
{
    std::unordered_set<char> invalid{delims.begin(), delims.end()};

    mmap_file mmap_file{path};
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
