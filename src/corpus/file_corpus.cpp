/**
 * @file file_corpus.cpp
 * @author Sean Massung
 */

#include "corpus/file_corpus.h"
#include "io/parser.h"

namespace meta
{
namespace corpus
{

file_corpus::file_corpus(const std::string& prefix, const std::string& doc_list,
                         std::string encoding)
    : corpus{std::move(encoding)}, cur_{0}, prefix_{prefix}
{
    io::parser psr{doc_list, "\n"};
    uint64_t idx = 0;
    while (psr.has_next())
    {
        std::string line = psr.next();
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
    document doc{prefix_ + docs_[cur_].first, doc_id{cur_}, docs_[cur_].second};
    doc.encoding(encoding());
    ++cur_;
    return doc;
}

uint64_t file_corpus::size() const
{
    return docs_.size();
}
}
}
