/**
 * @file libsvm_corpus.cpp
 * @author Chase Geigle
 */

#include "corpus/libsvm_corpus.h"
#include "io/filesystem.h"
#include "io/libsvm_parser.h"
#include "util/shim.h"

namespace meta
{
namespace corpus
{

const util::string_view libsvm_corpus::id = "libsvm-corpus";

libsvm_corpus::libsvm_corpus(const std::string& file,
                             label_type type /* = label_type::CLASSIFICATION */,
                             uint64_t num_lines /* = 0 */)
    : corpus{"utf-8"},
      cur_id_{0},
      lbl_type_{type},
      num_lines_{num_lines},
      input_{file}
{
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

    // read first document
    std::getline(input_, next_content_);
}

bool libsvm_corpus::has_next() const
{
    return !next_content_.empty();
}

document libsvm_corpus::next()
{
    class_label label{"[none]"};

    auto lbl = io::libsvm_parser::label(next_content_);
    auto mdata = next_metadata();
    switch (lbl_type_)
    {
        case label_type::CLASSIFICATION:
            label = std::move(lbl);
            break;
        case label_type::REGRESSION:
            double response = std::stod(static_cast<const std::string&>(lbl));
            mdata.insert(mdata.begin(), metadata::field{response});
            break;
    }

    document doc{cur_id_++, label};
    doc.content(next_content_, encoding());
    doc.mdata(std::move(mdata));

    // buffer in next document
    std::getline(input_, next_content_);

    return doc;
}

metadata::schema libsvm_corpus::schema() const
{
    auto schema = corpus::schema();
    if (lbl_type_ == label_type::REGRESSION)
    {
        schema.insert(
            schema.begin(),
            metadata::field_info{"response", metadata::field_type::DOUBLE});
    }
    return schema;
}

uint64_t libsvm_corpus::size() const
{
    return num_lines_;
}

template <>
std::unique_ptr<corpus> make_corpus<libsvm_corpus>(util::string_view prefix,
                                                   util::string_view dataset,
                                                   const cpptoml::table& config)
{
    // string_view doesn't have operator+ overloads...
    auto filename = prefix.to_string();
    filename += "/";
    filename.append(dataset.data(), dataset.size());
    filename += "/";
    filename.append(dataset.data(), dataset.size());
    filename += ".dat";

    auto lbl_type = libsvm_corpus::label_type::CLASSIFICATION;
    auto cfg_lbl_type = config.get_as<std::string>("label-type");
    if (cfg_lbl_type)
    {
        if (*cfg_lbl_type == "regression")
        {
            lbl_type = libsvm_corpus::label_type::REGRESSION;
        }
        else if (*cfg_lbl_type != "classification")
        {
            throw corpus_exception{"unrecognized label-type: " + *cfg_lbl_type};
        }
    }

    auto lines = config.get_as<int64_t>("num-lines");
    if (!lines)
        return make_unique<libsvm_corpus>(filename, lbl_type);
    else
        return make_unique<libsvm_corpus>(filename, lbl_type,
                                          static_cast<uint64_t>(*lines));
}
}
}
