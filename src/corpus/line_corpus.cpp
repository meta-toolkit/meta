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
      cur_id_{0},
      num_lines_{num_lines},
      parser_{file, "\n"}
{
    // init class label info
    if (filesystem::file_exists(file + ".labels"))
    {
        class_parser_ = make_unique<io::parser>(file + ".labels", "\n");
        if (num_lines_ == 0)
            num_lines_ = filesystem::num_lines(file + ".labels");
    }

    // if we couldn't determine the number of lines in the constructor and the
    // two optional files don't exist, we have to count newlines here
    if (num_lines_ == 0)
        num_lines_ = filesystem::num_lines(file);
}

bool line_corpus::has_next() const
{
    return parser_.has_next();
}

document line_corpus::next()
{
    class_label label{"[none]"};

    if (class_parser_)
        label = class_label{class_parser_->next()};

    document doc{cur_id_++, label};
    doc.content(parser_.next(), encoding());
    doc.mdata(next_metadata());

    return doc;
}

uint64_t line_corpus::size() const
{
    return num_lines_;
}
}
}
