/**
 * @file line_corpus.cpp
 * @author Sean Massung
 */

#include <algorithm>

#include "corpus/line_corpus.h"
#include "io/filesystem.h"
#include "io/parser.h"
#include "util/shim.h"

namespace meta
{
namespace corpus
{

const util::string_view line_corpus::id = "line-corpus";

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

    if (num_lines_ == 0 && filesystem::file_exists(file + ".numdocs"))
    {
        try
        {
            num_lines_ = std::stoul(filesystem::file_text(file + ".numdocs"));
        }
        catch (const std::exception& ex)
        {
            throw corpus_exception{"Malformed numdocs file " + file
                                   + ".numdocs: " + ex.what()};
        }
    }

    // if we couldn't determine the number of lines in the constructor and the
    // optional files don't exist, we have to count newlines here
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

template <>
std::unique_ptr<corpus> make_corpus<line_corpus>(util::string_view prefix,
                                                 util::string_view dataset,
                                                 const cpptoml::table& config)
{
    auto encoding = config.get_as<std::string>("encoding").value_or("utf-8");

    // string_view doesn't have operator+ overloads...
    auto filename = prefix.to_string();
    filename += "/";
    filename.append(dataset.data(), dataset.size());
    filename += "/";
    filename.append(dataset.data(), dataset.size());
    filename += ".dat";

    auto lines = config.get_as<int64_t>("num-lines");
    if (!lines)
        return make_unique<line_corpus>(filename, encoding);
    else
        return make_unique<line_corpus>(filename, encoding,
                                        static_cast<uint64_t>(*lines));
}
}
}
