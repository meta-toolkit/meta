/**
 * @file line_corpus.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include "io/parser.h"
#include "io/mmap_file.h"
#include "corpus/line_corpus.h"

namespace meta {
namespace corpus {

line_corpus::line_corpus(const std::string & file):
    _cur{0},
    _num_lines{0},
    _parser{file, "\n"}
{
    std::cerr << "Counting number of lines in corpus file..." << std::endl;
    _num_lines = common::num_lines(file);
}

bool line_corpus::has_next() const
{
    return _parser.has_next();
}

document line_corpus::next()
{
    document d{"[NONE]", doc_id{_cur++}};
    d.set_content(_parser.next());
    return d;
}

uint64_t line_corpus::size() const
{
    return _num_lines;
}

}
}
