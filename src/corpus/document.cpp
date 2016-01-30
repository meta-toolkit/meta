/**
 * @file document.cpp
 * @author Sean Massung
 */

#include "meta/corpus/corpus.h"
#include "meta/corpus/document.h"
#include "meta/util/mapping.h"

namespace meta
{
namespace corpus
{

document::document(doc_id d_id, const class_label& label)
    : d_id_{d_id}, label_{label}, encoding_{"utf-8"}
{
    // nothing
}

const class_label& document::label() const
{
    return label_;
}

void document::content(const std::string& content,
                       const std::string& encoding /* = "utf-8" */)
{
    content_ = content;
    encoding_ = encoding;
}

void document::encoding(const std::string& encoding)
{
    encoding_ = encoding;
}

const std::string& document::content() const
{
    if (content_)
        return *content_;
    throw corpus_exception{"there is no content for the requested document"};
}

const std::string& document::encoding() const
{
    return encoding_;
}

doc_id document::id() const
{
    return d_id_;
}

bool document::contains_content() const
{
    return static_cast<bool>(content_);
}

void document::label(class_label label)
{
    label_ = label;
}

const std::vector<metadata::field>& document::mdata() const
{
    return mdata_;
}

void document::mdata(std::vector<metadata::field>&& metadata)
{
    mdata_ = std::move(metadata);
}
}
}
