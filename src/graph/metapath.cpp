/**
 * @file metapath.cpp
 * @author Sean Massung
 */

#include <sstream>
#include "graph/metapath.h"

namespace meta
{
namespace graph
{
metapath::metapath(const std::string& str)
{
    std::istringstream stream{str};
    std::string buffer;
    bool on_node = true;
    while (stream >> buffer)
    {
        if (on_node)
            path_.push_back(buffer);
        else
        {
            if (buffer == "->")
                trans_.push_back(direction::forward);
            else if (buffer == "<-")
                trans_.push_back(direction::backward);
            else if (buffer == "--")
                trans_.push_back(direction::none);
            else
                throw metapath_exception{"invalid direction string"};
        }
        on_node = !on_node;
    }
}

const std::string& metapath::operator[](uint64_t idx) const
{
    if (idx >= size())
        throw metapath_exception{"idx out of range"};

    return path_.at(idx);
}

metapath::direction metapath::edge_dir(uint64_t idx) const
{
    if (idx >= size() - 1)
        throw metapath_exception{"idx out of range"};

    return trans_.at(idx);
}

uint64_t metapath::size() const
{
    return path_.size();
}
}
}
