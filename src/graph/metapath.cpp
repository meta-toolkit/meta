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
metapath::metapath(const std::string& str) : text_{str}
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

    if (path_.size() < 2)
        throw metapath_exception{"not a complete metapath"};
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

void metapath::reverse()
{
    // reverse internal representations

    std::reverse(path_.begin(), path_.end());
    std::reverse(trans_.begin(), trans_.end());
    for (auto& tran : trans_)
    {
        if (tran == direction::forward)
            tran = direction::backward;
        else if (tran == direction::backward)
            tran = direction::forward;
    }

    // create new string representation

    text_ = "";
    for (size_t i = 0; i < size() - 1; ++i)
    {
        text_ += path_[i];
        if (trans_[i] == direction::forward)
            text_ += " -> ";
        else if (trans_[i] == direction::backward)
            text_ += " <- ";
        else
            text_ += " -- ";
    }
    text_ += path_[size() - 1];
}

std::string metapath::text() const
{
    return text_;
}

uint64_t metapath::size() const
{
    return path_.size();
}
}
}
