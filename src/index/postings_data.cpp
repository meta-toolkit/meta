/**
 * @file postings_data.cpp
 * @author Sean Massung
 */

#include <sstream>
#include "index/postings_data.h"
#include "util/common.h"

namespace meta {
namespace index {

postings_data::postings_data(term_id t_id):
    _t_id(t_id)
{ /* nothing */ }

postings_data::postings_data(const std::string & raw_data)
{
    std::istringstream iss{raw_data};
    iss >> *this;
}

void postings_data::merge_with(const postings_data & other)
{
    for(auto & p: other._counts)
        _counts[p.first] += p.second;
}

void postings_data::increase_count(doc_id d_id, uint64_t amount)
{
    _counts[d_id] += amount;
}

uint64_t postings_data::count(doc_id d_id) const
{
    return common::safe_at(_counts, d_id);
}

const std::unordered_map<doc_id, uint64_t> & postings_data::counts() const
{
    return _counts;
}

bool postings_data::operator<(const postings_data & other) const
{
    return term() < other.term();
}

term_id postings_data::term() const
{
    return _t_id;
}

uint64_t postings_data::idf() const
{
    return _counts.size();
}

std::istream & operator>>(std::istream & in, postings_data & pd)
{
    std::string buffer;
    std::getline(in, buffer);
    std::istringstream iss{buffer};

    iss >> pd._t_id;
    pd._counts.clear();

    doc_id d_id;
    uint64_t count;
    while(iss.good())
    {
        iss >> d_id;
        iss >> count;
        pd._counts[d_id] += count;
    }

    return in;
}

std::ostream & operator<<(std::ostream & out, postings_data & pd)
{
    if(pd._counts.empty())
        return out;

    out << pd._t_id;
    for(auto & p: pd._counts)
    {
        out << " " << p.first;
        out << " " << p.second;
    }
    out << "\n";
    return out;
}

}
}
