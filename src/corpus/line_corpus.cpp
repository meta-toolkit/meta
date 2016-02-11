/**
 * @file line_corpus.cpp
 * @author Sean Massung
 */

#include <algorithm>

#include "meta/corpus/line_corpus.h"
#include "meta/io/filesystem.h"
#include "meta/util/shim.h"

namespace meta
{
namespace corpus
{

const util::string_view line_corpus::id = "line-corpus";

line_corpus::line_corpus(const std::string& file, std::string encoding,
                         uint64_t num_docs /* = 0 */)
    : corpus{std::move(encoding)},
      cur_id_{0},
      num_lines_{num_docs},
      infile_{file}
{
    // init class label info
    if (filesystem::file_exists(file + ".labels"))
    {
        class_infile_ = make_unique<std::ifstream>(file + ".labels");
        if (num_lines_ == 0)
            num_lines_ = filesystem::num_lines(file + ".labels");
    }

    // if we couldn't determine the number of lines in the constructor, we have
    // to count newlines
    if (num_lines_ == 0)
        num_lines_ = filesystem::num_lines(file);
}

bool line_corpus::has_next() const
{
    return cur_id_ < size();
}

document line_corpus::next()
{
    class_label label{"[none]"};

    if (class_infile_)
        *class_infile_ >> label;

    document doc{cur_id_++, label};
    std::string content;
    if (!std::getline(infile_, content))
        throw corpus_exception{"error parsing line_corpus line "
                               + std::to_string(cur_id_)};

    doc.content(content, encoding());
    auto mdata = next_metadata();
    if (store_full_text())
        mdata.insert(mdata.begin(), metadata::field{doc.content()});
    doc.mdata(std::move(mdata));

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

    auto lines = config.get_as<int64_t>("num-docs");
    if (!lines)
        return make_unique<line_corpus>(filename, encoding);
    else
        return make_unique<line_corpus>(filename, encoding,
                                        static_cast<uint64_t>(*lines));
}
}
}
