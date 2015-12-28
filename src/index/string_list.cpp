/**
 * @file string_list.cpp
 * @author Chase Geigle
 */

#include "meta/index/string_list.h"

namespace meta
{
namespace index
{

string_list::string_list(const std::string& path)
    : string_file_{path}, index_{path + "_index"}
{
    // nothing
}

const char* string_list::at(uint64_t idx) const
{
    return string_file_.begin() + index_[idx];
}

uint64_t string_list::size() const
{
    return index_.size();
}
}
}
