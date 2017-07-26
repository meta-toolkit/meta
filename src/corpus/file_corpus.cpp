/**
 * @file file_corpus.cpp
 * @author Sean Massung
 */

#include <fstream>

#include "meta/corpus/file_corpus.h"
#include "meta/io/filesystem.h"
#include "meta/utf/utf.h"
#include "meta/util/shim.h"

namespace meta
{
namespace corpus
{

const util::string_view file_corpus::id = "file-corpus";

file_corpus::file_corpus(const std::string& prefix, const std::string& doc_list,
                         std::string encoding)
    : corpus{std::move(encoding)}, cur_{0}, prefix_{prefix}
{
    std::ifstream input{doc_list};
    uint64_t idx = 0;
    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty())
            throw corpus_exception{"empty line in corpus list: line #"
                                   + std::to_string(idx + 1)};
        size_t space = line.find_first_of(" ");
        if (space != std::string::npos)
        {
            std::string file{line.substr(space + 1)};
            class_label label{line.substr(0, space)};
            docs_.emplace_back(std::make_pair(file, label));
        }
        else
        {
            throw corpus_exception{"document list needs class label prefix "
                                   "(add [none] if there are no labels)"};
        }

        ++idx;
    }
}

bool file_corpus::has_next() const
{
    return cur_ < docs_.size();
}

document file_corpus::next()
{
    document doc{doc_id{cur_}, docs_[cur_].second};

    if (!filesystem::file_exists(prefix_ + docs_[cur_].first))
        throw corpus_exception{"file \"" + docs_[cur_].first
                               + "\" does not exist"};

    doc.content(filesystem::file_text(prefix_ + docs_[cur_].first), encoding());

    auto mdata = next_metadata();
    if (store_full_text())
        mdata.insert(mdata.begin(), metadata::field{doc.content()});

    // add "path" metadata manually
    mdata.insert(mdata.begin(), metadata::field{prefix_ + docs_[cur_].first});
    doc.mdata(std::move(mdata));

    ++cur_;
    return doc;
}

uint64_t file_corpus::size() const
{
    return docs_.size();
}

metadata::schema_type file_corpus::schema() const
{
    auto schema = corpus::schema();
    schema.insert(schema.begin(),
                  metadata::field_info{"path", metadata::field_type::STRING});
    return schema;
}

template <>
std::unique_ptr<corpus> make_corpus<file_corpus>(util::string_view prefix,
                                                 util::string_view dataset,
                                                 const cpptoml::table& config)
{
    auto encoding = config.get_as<std::string>("encoding").value_or("utf-8");

    auto file_list = config.get_as<std::string>("list");
    if (!file_list)
        throw corpus_exception{"list missing from corpus configuration file"};

    // string_view doesn't have operator+ overloads...
    auto folder = prefix.to_string();
    folder += "/";
    folder.append(dataset.data(), dataset.size());
    folder += "/";

    auto file = folder + *file_list + "-full-corpus.txt";
    return make_unique<file_corpus>(folder, file, encoding);
}
}
}
