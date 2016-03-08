/**
 * @file trec_corpus.cpp
 * @author Sean Massung
 */

#include <iostream>

#include "meta/corpus/trec_corpus.h"
#include "meta/io/filesystem.h"
#include "meta/io/gzstream.h"
#include "meta/util/shim.h"

namespace meta
{
namespace corpus
{

const util::string_view trec_corpus::id = "trec-corpus";

trec_corpus::trec_corpus(const std::string& prefix,
                         const std::string& file_list_path,
                         std::string encoding, uint64_t num_docs,
                         const std::string& doc_tag,
                         const std::string& name_tag,
                         const std::vector<std::string>& skip_tags)
    : corpus{std::move(encoding)},
      prefix_{prefix},
      cur_id_{0},
      num_docs_{num_docs},
      file_idx_{0},
      buf_idx_{std::string::npos},
      doc_start_tag_{"<" + doc_tag + ">"},
      doc_end_tag_{"</" + doc_tag + ">"},
      name_start_tag_{"<" + name_tag + ">"},
      name_end_tag_{"</" + name_tag + ">"},
      skip_tags_{skip_tags}
{
    std::string path;
    std::ifstream file_list{file_list_path};
    while (std::getline(file_list, path))
    {
        if (path.empty())
            continue;
        filenames_.emplace_back(std::move(path));
    }

    if (filenames_.empty())
        throw corpus_exception{"empty trec_corpus file list " + file_list_path};

    for (auto& tag : skip_tags_)
        tag = "</" + tag + ">";

    advance();
}

bool trec_corpus::has_next() const
{
    return cur_id_ != num_docs_;
}

void trec_corpus::advance()
{
    buf_idx_ = buffer_.find(doc_start_tag_, buf_idx_);

    if (buf_idx_ == std::string::npos)
    {
        auto gz
            = make_unique<io::gzifstream>(prefix_ + filenames_[file_idx_++]);
        std::stringstream ss;
        ss << gz->rdbuf();
        buffer_ = ss.str();
        buf_idx_ = buffer_.find(doc_start_tag_);
    }
}

document trec_corpus::next()
{
    document doc{cur_id_++};

    auto doc_end = buffer_.find(doc_end_tag_, buf_idx_);
    auto name_start
        = buffer_.find(name_start_tag_, buf_idx_) + name_start_tag_.size();
    auto name_end = buffer_.find(name_end_tag_, name_start);
    auto docno = buffer_.substr(name_start, name_end - name_start);

    auto text_start = name_end + name_end_tag_.size();
    for (const auto& tag : skip_tags_)
    {
        auto tag_end = buffer_.find(tag, buf_idx_);
        if (tag_end == std::string::npos)
            throw corpus_exception{
                "invalid skip tag specified for trec_corpus: " + tag};

        tag_end += tag.size();
        if (tag_end > text_start)
            text_start = tag_end;
    }

    auto text = buffer_.substr(text_start, doc_end - text_start);
    doc.content(text, encoding());

    auto mdata = next_metadata();
    if (store_full_text())
        mdata.insert(mdata.begin(), metadata::field{doc.content()});

    // add "path" metadata manually
    mdata.insert(mdata.begin(),
                 metadata::field{filenames_[file_idx_ - 1] + "/" + docno});
    doc.mdata(std::move(mdata));

    buf_idx_ = doc_end;

    if (has_next())
        advance();

    return doc;
}

uint64_t trec_corpus::size() const
{
    return num_docs_;
}

metadata::schema trec_corpus::schema() const
{
    auto schema = corpus::schema();
    schema.insert(schema.begin(),
                  metadata::field_info{"path", metadata::field_type::STRING});
    return schema;
}

template <>
std::unique_ptr<corpus> make_corpus<trec_corpus>(util::string_view prefix,
                                                 util::string_view dataset,
                                                 const cpptoml::table& config)
{
    auto encoding = config.get_as<std::string>("encoding").value_or("utf-8");
    auto doc_tag = config.get_as<std::string>("doc-tag").value_or("DOC");
    auto name_tag = config.get_as<std::string>("name-tag").value_or("DOCNO");

    auto num_docs = config.get_as<int64_t>("num-docs");
    if (!num_docs)
        throw corpus_exception{
            "num-docs config param required for trec_corpus"};

    // string_view doesn't have operator+ overloads...
    auto file_list_prefix = prefix.to_string();
    file_list_prefix += "/";
    file_list_prefix.append(dataset.data(), dataset.size());
    file_list_prefix += "/";

    auto default_file_list = dataset.to_string() + "-full-corpus.txt";
    auto file_list
        = file_list_prefix
          + config.get_as<std::string>("file-list").value_or(default_file_list);

    std::vector<std::string> skip_tags;
    auto skips = config.get_array("skip-tags");
    if (skips)
    {
        for (const auto& t : skips->array_of<std::string>())
            skip_tags.push_back(t->get());
    }

    return make_unique<trec_corpus>(file_list_prefix, file_list, encoding,
                                    static_cast<uint64_t>(*num_docs), doc_tag,
                                    name_tag, skip_tags);
}
}
}
