/**
 * @file parser.cpp
 */

#include "io/parser.h"

namespace meta {
namespace io {

parser::parser(const std::string & input, const std::string & delims,
        input_type in_type /* = File */):
    _idx{0},
    _invalid{delims.begin(), delims.end()}
{
    if(in_type == input_type::File)
    {
        _filename = input;
        _mmap_file = std::unique_ptr<io::mmap_file>{
            new io::mmap_file{input}
        };
        _data = _mmap_file->start();
        _size = _mmap_file->size();
    }
    else /* in_type == input_type::String */
    {
        _filename = "";
        _data = input.data();
        _size = input.size();
    }

    get_next();
}

void parser::get_next()
{
    _next = "";
    for(size_t i = _idx; i < _size; ++i)
    {
        if(_invalid.find(_data[i]) != _invalid.end())
        {
            if(_idx != i)
            {
                _next = std::string{_data + _idx, i - _idx};
                _idx = i + 1;
                return;
            }
            _idx = i + 1;
        }
    }

    // get last token if there is no delimiter at the EOF
    if(_idx != _size)
    {
        _next = std::string{_data + _idx, _size - _idx};
        _idx = _size;
    }
}

std::string parser::filename() const
{
    return _filename;
}

std::string parser::peek() const
{
    return _next;
}

std::string parser::next()
{
    std::string ret{_next};
    get_next();
    return ret;
}

bool parser::has_next() const
{
    return _next != "";
}

}
}
