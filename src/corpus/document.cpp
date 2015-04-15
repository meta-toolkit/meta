/**
 * @file document.cpp
 * @author Sean Massung
 */

#include "corpus/corpus.h"
#include "corpus/document.h"
#include "util/mapping.h"

namespace meta
{
namespace corpus
{

document::document(doc_id d_id, const class_label& label)
    : d_id_{d_id}, label_{label}, length_{0}, encoding_{"utf-8"}
{
}

void document::increment(const std::string& term, double amount)
{
    counts_[term] += amount;
    length_ += amount;
}

const class_label& document::label() const
{
    return label_;
}

uint64_t document::length() const
{
    return length_;
}

double document::count(const std::string& term) const
{
    return map::safe_at(counts_, term);
}

const std::unordered_map<std::string, double>& document::counts() const
{
    return counts_;
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
    throw corpus::corpus_exception{
        "there is no content for the requested document"};
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

const std::vector<metadata::field>& document::metadata() const
{
    return mdata_;
}

void document::metadata(std::vector<metadata::field>&& metadata)
{
    mdata_ = std::move(metadata);
}
}
}
