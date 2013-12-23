/**
 * @file line_corpus.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include "io/parser.h"
#include "io/mmap_file.h"
#include "corpus/line_corpus.h"
#include "util/filesystem.h"

namespace meta {
namespace corpus {

line_corpus::line_corpus(const std::string & file,
        uint64_t num_lines /* = 0 */):
    _cur{0},
    _num_lines{num_lines},
    _parser{file, "\n"}
{
    if(_num_lines == 0)
        _num_lines = filesystem::num_lines(file);
}

bool line_corpus::has_next() const
{
    return _parser.has_next();
}

document line_corpus::next()
{
    std::string content = _parser.next();
    size_t space = content.find_first_of(" ");
    std::string name = common::to_string(_cur) + "_" + content.substr(0, space);
    document d{name, doc_id{_cur++}, class_label{content.substr(0, space)}};
    d.set_content(content.substr(space + 1));
    return d;
}

uint64_t line_corpus::size() const
{
    return _num_lines;
}

}
}
