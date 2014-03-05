/**
 * @file document.cpp
 * @author Sean Massung
 */

#include "cluster/similarity.h"
#include "corpus/corpus.h"
#include "corpus/document.h"
#include "util/mapping.h"

namespace meta {
namespace corpus {

document::document(const std::string & path,
                   doc_id d_id,
                   const class_label & label):
    _path{path},
    _d_id{d_id},
    _label{label},
    _length{0},
    _encoding{"utf-8"}
{
    size_t idx = path.find_last_of("/") + 1;
    _name = path.substr(idx);
}

void document::increment(const std::string & term, double amount)
{
    _counts[term] += amount;
    _length += amount;
}

std::string document::path() const
{
    return _path;
}

const class_label & document::label() const
{
    return _label;
}

std::string document::name() const
{
    return _name;
}

uint64_t document::length() const
{
    return _length;
}

double document::count(const std::string & term) const
{
    return map::safe_at(_counts, term);
}

const std::unordered_map<std::string, double> & document::counts() const
{
    return _counts;
}

double document::cosine_similarity(const document & a, const document & b)
{
    return clustering::similarity::cosine_similarity(a._counts,
                                                     b._counts);
}

double document::jaccard_similarity(const document & a, const document & b)
{
    return clustering::similarity::jaccard_similarity(a._counts,
                                                      b._counts);
}

void document::set_content(const std::string& content,
                           const std::string& encoding /* = "utf-8" */)
{
    _content = content;
    _encoding = encoding;
}

void document::set_encoding(const std::string& encoding)
{
    _encoding = encoding;
}

const std::string & document::content() const
{
    if (_content)
        return *_content;
    throw corpus::corpus_exception{
        "there is no content for the requested document"};
}

const std::string& document::encoding() const
{
    return _encoding;
}

doc_id document::id() const
{
    return _d_id;
}

bool document::contains_content() const
{
    return static_cast<bool>(_content);
}

void document::set_label(class_label label)
{
    _label = label;
}

}
}
