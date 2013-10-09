/**
 * @file line_corpus.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include "io/parser.h"
#include "io/mmap_file.h"
#include "corpus/line_corpus.h"

namespace meta {
namespace corpus {

line_corpus::line_corpus(const std::string & file):
    _cur{0},
    _num_lines{lines(file)},
    _parser{file, "\n"}
{ /* nothing */ }

uint64_t line_corpus::lines(const std::string & file) const
{
    io::mmap_file f{file};
    return std::count(f.start(), f.start() + f.size(), '\n');
}

bool line_corpus::has_next() const
{
    return _parser.has_next();
}

document line_corpus::next()
{
    document d{"", doc_id{_cur++}};
    d.set_content(_parser.next());
    return d;
}

uint64_t line_corpus::size() const
{
    return _num_lines;
}

}
}
