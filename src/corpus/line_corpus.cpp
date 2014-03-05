/**
 * @file line_corpus.cpp
 * @author Sean Massung
 */

#include <algorithm>

#include "corpus/line_corpus.h"
#include "io/parser.h"
#include "util/filesystem.h"
#include "util/shim.h"

namespace meta
{
namespace corpus
{

line_corpus::line_corpus(const std::string& file, std::string encoding,
                         uint64_t num_lines /* = 0 */)
    : corpus{std::move(encoding)},
      _cur_id{0},
      _num_lines{num_lines},
      _parser{file, "\n"}
{
    // init class label info
    if (filesystem::file_exists(file + ".labels"))
    {
        _class_parser = make_unique<io::parser>(file + ".labels", "\n");
        if (_num_lines == 0)
            _num_lines = filesystem::num_lines(file + ".labels");
    }

    // init class label info
    if (filesystem::file_exists(file + ".names"))
    {
        _name_parser = make_unique<io::parser>(file + ".names", "\n");
        if (_num_lines == 0)
            _num_lines = filesystem::num_lines(file + ".names");
    }

    // if we couldn't determine the number of lines in the constructor and the
    // two optional files don't exist, we have to count newlines here
    if (_num_lines == 0)
        _num_lines = filesystem::num_lines(file);
}

bool line_corpus::has_next() const
{
    return _parser.has_next();
}

document line_corpus::next()
{
    class_label label{"[none]"};
    std::string name{"[none]"};

    if (_class_parser)
        label = class_label{_class_parser->next()};

    if (_name_parser)
        name = _name_parser->next();

    document doc{name, _cur_id++, label};
    doc.set_content(_parser.next(), encoding());

    return doc;
}

uint64_t line_corpus::size() const
{
    return _num_lines;
}
}
}
