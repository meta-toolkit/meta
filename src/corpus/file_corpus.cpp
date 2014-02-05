/**
 * @file file_corpus.cpp
 * @author Sean Massung
 */

#include "corpus/file_corpus.h"
#include "io/parser.h"

namespace meta {
namespace corpus {

file_corpus::file_corpus(const std::string & prefix,
        const std::string & doc_list):
    _cur{0},
    _prefix{prefix}
{
    io::parser psr{doc_list, "\n"};
    uint64_t idx = 0;
    while(psr.has_next())
    {
        std::string line = psr.next();
        size_t space = line.find_first_of(" ");
        if(space != std::string::npos)
        {
            std::string file{line.substr(space + 1)};
            class_label label{line.substr(0, space)};
            _docs.emplace_back(std::make_pair(file, label));
        }
        else
            _docs.emplace_back(std::make_pair(line, class_label{""}));
        ++idx;
    }
}

bool file_corpus::has_next() const
{
    return _cur < _docs.size();
}

document file_corpus::next()
{
    document doc{_prefix + _docs[_cur].first, doc_id{_cur}, _docs[_cur].second};
    ++_cur;
    return doc;
}

uint64_t file_corpus::size() const
{
    return _docs.size();
}

}
}
