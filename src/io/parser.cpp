/**
 * @file parser.cpp
 */

#include "io/mmap_file.h"
#include "io/parser.h"
#include "util/shim.h"

namespace meta {
namespace io {

parser::parser(const std::string & input, const std::string & delims,
        input_type in_type /* = File */):
    _idx{0}
{
    // initialize delimiter array
    _invalid.fill(false);
    for(const auto & ch: delims)
        _invalid[static_cast<uint8_t>(ch)] = true;

    // determine whether we're parsing an mmap_file or a std::string
    if(in_type == input_type::File)
    {
        _filename = input;
        _mmap_file = make_unique<io::mmap_file>(input);
        _data = _mmap_file->begin();
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

parser::parser(parser&&) = default;
parser::~parser() = default;
parser& parser::operator=(parser&&) = default;

void parser::get_next()
{
    _next = "";
    for(size_t i = _idx; i < _size; ++i)
    {
        if(_invalid[static_cast<uint8_t>(_data[i])])
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
