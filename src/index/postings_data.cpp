/**
 * @file postings_data.cpp
 * @author Sean Massung
 */

#include <sstream>
#include "index/postings_data.h"

namespace meta {
namespace index {

postings_data::postings_data(term_id t_id):
    _t_id(t_id), _counts(std::unordered_map<doc_id, uint32_t>{})
{ /* nothing */ }

postings_data::postings_data(const std::string & raw_data)
{
    std::istringstream iss{raw_data};
    iss >> _t_id;

    // TODO
}

std::string postings_data::to_string() const
{
    std::string ret;

    // TODO
    
    return ret;
}

void postings_data::merge_with(const postings_data & other)
{
    for(auto & p: other._counts)
        _counts[p.first] += p.second;
}

void postings_data::increase_count(doc_id d_id, uint32_t amount)
{
    _counts[d_id] += amount;
}

bool postings_data::operator<(const postings_data & other) const
{
    return term() < other.term();
}

term_id postings_data::term() const
{
    return _t_id;
}

uint32_t postings_data::idf() const
{
    return _counts.size();
}

}
}
